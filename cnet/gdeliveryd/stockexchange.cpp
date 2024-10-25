#include "log.h"
#include "mapuser.h"
#include "stockexchange.h"
#include "dbstockbalance.hrp"
#include "dbstockcancel.hrp"
#include "gamedbclient.hpp"
#include "dbstockload.hrp"
#include "dbsetcashpassword.hrp"

namespace GNET
{

bool StockExchange::Initialize()
{
	IntervalTimer::Attach(this,500000/IntervalTimer::Resolution());
	return true;
}
void UserAccount::OnCommission(int& new_cash, int& new_money)
{
	UnLock();
	GetFreeFund(new_cash,new_money);
}

bool UserAccount::OnCancel(unsigned int tid, short result, int& price, int new_cash, int new_money)
{
	for(std::vector<StockOrder>::iterator it=orders.begin(),ie=orders.end();it!=ie;++it)
	{
		if(tid==it->tid)
		{
			StockLog log;
			log.tid = tid;
			log.time = Timer::GetTime();
			log.volume = it->volume;
			log.cost = it->price;
			log.result = result;
			UpdateLog(log);

			price = it->price;
			orders.erase(it);
			break;
		}
	}
	UpdateAccount(new_cash, new_money);
	UnLock();
	return true;
}

bool UserAccount::OnBalance(unsigned int tid,int ack_volume,int volume_left,int new_cash,int new_money)
{
	bool finished = false;
	for(std::vector<StockOrder>::iterator it=orders.begin(),ie=orders.end();it!=ie;++it)
	{
		if(tid!=it->tid)
			continue;
		it->volume -= ack_volume;
		DEBUG_PRINT("UserAccount::OnBalance, volume=%d,ack_volume=%d", it->volume, ack_volume);
		if(it->volume!=volume_left)
			Log::log(LOG_ERR,"Stock::OnBalance, order volume out of sync %d!=%d(database)",it->volume,volume_left);
		if(it->volume<=0)
		{
			finished = true;
			orders.erase(it);
		}
		if(it->price>0)
			cash_sell += (int)(STOCK_TAX_RATE*ack_volume*100);
		else
			cash_buy += ack_volume*100;
		break;
	}
	cash = new_cash;
	money = new_money;
	UnLock();
	return finished;
}

bool UserAccount::OnTransaction(int retcode, int new_cash, int new_money)
{
	if(retcode==ERR_SUCCESS)
		UpdateAccount(new_cash,new_money);
	UnLock();
	return true;
}

void UserAccount::UpdateLog(StockLog& log)
{
	logs.push_back(log);
	if(logs.size()>80)
		logs.erase(logs.begin());
}
void UserAccount::GetFreeFund(int& _cash, int& _money)
{
	_cash = _money = 0;
	for(std::vector<StockOrder>::iterator it=orders.begin(),ie=orders.end();it!=ie;++it)
	{
		if(it->price>0)
			_cash += it->volume*100;
		else
			_money += it->volume*it->price;
	}
	_cash  = cash - (int)(STOCK_TAX_RATE*_cash);
	_money = money + (int)(STOCK_TAX_RATE*_money);
}

void UserAccount::GetMaxFund(int& _cash, int& _money)
{
	_cash = _money = 0;
	for(std::vector<StockOrder>::iterator it=orders.begin(),ie=orders.end();it!=ie;++it)
	{
		if(it->price>0)
			_money += it->volume*it->price;
		else
			_cash += it->volume*100;
	}
	_cash  += cash;
	_money += money;
}

bool UserAccount::CashUnlock(const Octets& cash_pwd)
{
	if (!IsCashLocked()) return true;
	if (cash_pwd == cash_password)
	{	
		passtime = Timer::GetTime();
		ClearFlag(FLG_PWDLOCKED);
		return true;	
	}
	return false;
}

void UserAccount::CashLock()
{
	if (cash_password.size() > 0)	
		SetFlag(FLG_PWDLOCKED);
}

bool StockExchange::Update()
{
	if(status != EXCHANGE_TRADING) 
		return true;
	for(PendingList::iterator it=pending.begin();it!=pending.end();)
	{
		if(it->status==BALANCE_DONE)
		{
			it = pending.erase(it);
			DEBUG_PRINT("StockExchange::Erase pending, size=%d", pending.size());
			continue;
		}
		if(it->status!=COMMISSION_DONE)
		{
			++it;
			continue;
		}
		DEBUG_PRINT("StockExchange::Update, status=%d, it->order.tid=%d, size=%d", it->status, 
				it->order.tid, pending.size());

		if(LockAccount(it->order.userid))
		{
			if(it->ack_money>0 && it->order.volume)
			{
				trade_price = it->ack_money/it->order.volume;
				trade_volume += it->order.volume;
				trade_amount += it->ack_money/100;
			}
			DBStockBalanceArg arg(it->order.tid, it->order.userid, it->order.volume, it->ack_money);
			DBStockBalance* rpc = (DBStockBalance*) Rpc::Call( RPC_DBSTOCKBALANCE,arg);
			GameDBClient::GetInstance()->SendProtocol(rpc);
			it->status = BALANCE_WAIT;
			break;
		}
		++it;
	}
	CheckTimeout();
	return true;
}

void StockExchange::CheckTimeout()
{
	static int _tick = 0;
	time_t now = Timer::GetTime();
	if((_tick++)%1200 == 0)
	{
		DEBUG_PRINT("StockStatus:: uptime=%d,trade_price=%d,order_volume=%d,trade_volume=%d,trade_amount=%d,idpool=%d",
			now-starttime, trade_price, order_volume, trade_volume, trade_amount, idpool.size()); 
	}
	if(now>=checktime)
	{
		checktime = now+1200;
		bool send = false;
		for(OrderMap::iterator it=orders.begin();it!=orders.end();++it)
		{
			time_t endtime = it->second.time+MAX_ORDER_TIMEOUT;
			if(endtime<=now && it->second.volume)
			{
				if(send)
				{
					checktime = now;
					break;
				}
				if(!IsTrading(it->second.tid) && LockAccount(it->second.userid))
				{
					CancelOrder(it->second, STOCK_ORDER_TIMEOUT);
					send = true;
				}
				else
					checktime = now;
			}
			if(endtime<checktime)
				checktime = endtime;
		}
		checktime += 10;
	}
}

bool StockExchange::IsTrading(unsigned int tid)
{
	for(PendingList::iterator it=pending.begin(),ie=pending.end();it!=ie;++it)
		if(it->order.tid == tid)
			return true;   // 正在进行交易的订单不能取消
	return false;
}

void StockExchange::CancelOrder(StockOrder& order, int result)
{
	ReduceVolume(order.price, order.volume);
	DBStockCancelArg arg(order.userid, order.tid, order.volume, result);
	DBStockCancel* rpc = (DBStockCancel*) Rpc::Call( RPC_DBSTOCKCANCEL,arg);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	order.volume = 0;
}

void StockExchange::OnLogin(int userid, int cash, int money, int cash_buy, int cash_sell, std::vector<StockLog>& logs, Octets &cash_pwd)
{
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		it = accounts.insert(std::make_pair(userid, UserAccount(userid))).first;
	DEBUG_PRINT("StockExchange::OnLogin, userid=%d,cash=%d,money=%d,orders=%d,cash_pwd=%d",
		userid,cash,money,it->second.OrderSize(),cash_pwd.size());
	it->second.UpdateAccount(cash,money,cash_buy,cash_sell);
	it->second.InitCashPassword(cash_pwd);
	it->second.logs.swap(logs);
}

void StockExchange::OnLogout(int userid)
{
	AccountMap::iterator it = accounts.find(userid);
	if(it!=accounts.end() && it->second.OrderSize()==0)
		accounts.erase(userid);
}

bool StockExchange::TradeLog(int userid, std::vector<StockOrder>& orders, std::vector<StockLog>& logs)
{
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return false;
	logs = it->second.logs;
	orders = it->second.orders;
	return true;
}

bool StockExchange::LockAccount(int userid)
{
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return false;
	return it->second.Lock();
}

bool StockExchange::GetAccount(int userid, int& cash, int& money, char& locked, std::vector<StockPrice>& list)
{
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return false;
	it->second.GetFreeFund(cash, money);
	locked = it->second.IsCashLocked() ? 1 : 0;
	GetPrice(list);
	return true;
}

void StockExchange::GetPrice(std::vector<StockPrice>& list)
{
	int n = 0;
	VolumeMap::iterator ie=volumes.end(),it=volumes.upper_bound(0);
	for(;it!=ie && ++n<=8;++it)
		list.insert(list.begin(), StockPrice(it->first, it->second));
	n = 0;
	it = volumes.upper_bound(-MAX_STOCK_PRICE);
	for(;it!=ie && ++n<=8 && it->first<0;++it)
		list.push_back(StockPrice(it->first, it->second));
}

int StockExchange::PreTransaction(int userid, char withdraw, int cash, int money, int pocket_cash, int pocket_money)
{
	if (IsCashLocked(userid))
		return ERR_STOCK_CASHLOCKED;

	if(status != EXCHANGE_TRADING) 
		return ERR_STOCK_CLOSED;
	if(cash<0 || money < 0)
		return ERR_STOCK_INVALIDINPUT;
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return ERR_STOCK_CLOSED;
	if(withdraw)
	{
		int _cash, _money;
		it->second.GetFreeFund(_cash, _money);
		if(money>MAX_PACKAGE_MONEY-pocket_money)
			return ERR_STOCK_OVERFLOW;
		if(_cash<cash || _money<money)
			return ERR_STOCK_NOTENOUGH;
	}
	else
	{
		int _cash, _money;
		if(pocket_cash<cash || pocket_money<money)
			return ERR_STOCK_NOTENOUGH;
		it->second.GetMaxFund(_cash, _money);
		if(money>MAX_PACKAGE_MONEY-_money)
			return ERR_STOCK_OVERFLOW;
		if(cash>MAX_ACCOUNT_CASH-_cash)
			return ERR_STOCK_OVERFLOW;

	}
	if(!it->second.Lock())
		return ERR_STOCK_ACCOUNTBUSY;
	return 0;
}

void StockExchange::PostTransaction(int userid, int retcode, int& cash, int& money)
{
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return;
	it->second.OnTransaction(retcode, cash, money);
	it->second.GetFreeFund(cash, money);
}

bool  StockExchange::PriceMatching(unsigned int tid, int price, int volume, int& ack_volume, int& ack_money, bool dryrun)
{
	ack_money = 0;
	ack_volume = volume;

	OrderMap::iterator it = price>0 ? orders.upper_bound(-MAX_STOCK_PRICE) : orders.upper_bound(0);
	while(it!=orders.end() && volume>0)
	{
		if(price<0 && it->first>-price)
			break;
		if(price>0 && (it->first>=0 || -it->first<price))
			break;
		if(it->second.volume>volume)
		{
			if(!dryrun)
			{
				StockOrder o(it->second);
				o.volume = volume;
				pending.push_back(PendingOrder(o, tid, it->first*volume));

				ReduceVolume(it->first,volume);
				it->second.volume  -= volume;
			}
			ack_money -= it->first*volume;
			volume = 0;
			break;
		}
		else if(it->second.volume>0)
		{
			if(!dryrun)
			{
				pending.push_back(PendingOrder(it->second, tid, it->first*it->second.volume));
			}
			ack_money -= it->first*it->second.volume;
			volume -= it->second.volume;
			if(!dryrun)
			{
				ReduceVolume(it->first,it->second.volume);
				it->second.volume  = 0;
			}
		}
		++it;
	}
	ack_volume -= volume;
	LOG_TRACE("PriceMatching, tid=%d, price=%d, volume=%d, ack_volume=%d, ack_money=%d. dryrun=%d",
			tid,price,volume, ack_volume, ack_money, dryrun);
	return true;
}

int StockExchange::PlaceOrder(int userid, int price, int volume, int& ack_volume, unsigned int& tid)
{
	if (IsCashLocked(userid))
		return ERR_STOCK_CASHLOCKED;

	if(status != EXCHANGE_TRADING) 
		return ERR_STOCK_CLOSED;
	if(price<=-MAX_STOCK_PRICE || price>=MAX_STOCK_PRICE || !price || (price%100))
		return ERR_STOCK_INVALIDINPUT;
	if(volume>MAX_STOCK_VOLUME || volume<=0)
		return ERR_STOCK_OVERFLOW;
	int64_t total = (int64_t)price*volume;
	if(total>MAX_PACKAGE_MONEY || total<-MAX_PACKAGE_MONEY)
		return ERR_STOCK_OVERFLOW;
	int cost = (int)total;

	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return ERR_STOCK_CLOSED;
	if(it->second.OrderSize()>=10)
		return ERR_STOCK_MAXCOMMISSION;

	int cash, money, max_cash, max_money;
	it->second.GetFreeFund(cash, money);
	it->second.GetMaxFund(max_cash, max_money);
	if(price>0)
	{
		int maxsell = it->second.cash_sell+(int)(STOCK_TAX_RATE*volume*100);
		if(maxsell<0 || maxsell>MAX_USER_CASH )
			return ERR_STOCK_OVERFLOW;
		if(cash<(int)(STOCK_TAX_RATE*volume*100))
			return ERR_STOCK_NOTENOUGH;
		if(max_money>MAX_PACKAGE_MONEY-cost)
			return ERR_STOCK_OVERFLOW;
		//卖元宝时要使用PriceMatching来判断是否金钱超上限
		int tmp_ack_money, tmp_ack_volume;
		PriceMatching(0, price, volume, tmp_ack_volume, tmp_ack_money, true);
		if(tmp_ack_money<0 || max_money>MAX_PACKAGE_MONEY-(volume-tmp_ack_volume)*price-tmp_ack_money)
			return ERR_STOCK_OVERFLOW;		
	}else 
	{
		int maxbuy = it->second.cash_buy+volume*100;
		if(maxbuy<0 || maxbuy>MAX_USER_CASH)
			return ERR_STOCK_OVERFLOW;
		if(money<-(int)(STOCK_TAX_RATE*cost))
			return ERR_STOCK_NOTENOUGH;
		if(max_cash>MAX_ACCOUNT_CASH-volume*100)
			return ERR_STOCK_OVERFLOW;
	}
	tid = ApplyTid();
	if(!tid)
		return ERR_STOCK_CLOSED;
	if(!it->second.Lock())
		return ERR_STOCK_ACCOUNTBUSY;
	int ack_money;
	ack_volume = 0;
	PriceMatching(tid, price, volume, ack_volume, ack_money);
	StockOrder o(tid, Timer::GetTime(), userid, price, ack_volume, COMMISSION_WAIT);
	if(ack_volume)
		pending.push_back(PendingOrder(o, tid, ack_money));
	return 0;
}

int  StockExchange::PreCancel(int userid, unsigned int tid, int price)
{
	if (IsCashLocked(userid))
		return ERR_STOCK_CASHLOCKED;

	if(status != EXCHANGE_TRADING) 
		return ERR_STOCK_CLOSED;

	OrderMap::iterator it = FindOrder(tid, price);
	if(it==orders.end() || !it->second.volume)
		return ERR_STOCK_NOTFOUND;
	if(it->second.userid!=userid)
		return ERR_STOCK_INVALIDINPUT;
	if(!LockAccount(userid))
		return ERR_STOCK_ACCOUNTBUSY;
	if(!it->second.volume)
		return ERR_STOCK_NOTFOUND;
	if(IsTrading(tid))
		return ERR_STOCK_ACCOUNTBUSY;

	CancelOrder(it->second, STOCK_ORDER_CANCEL);
	return 0;
}

unsigned int StockExchange::ApplyTid()
{
	if(idpool.size()>=MAX_STOCKORDER_ID)
		return 0;  // 无可用tid
	do{
		nextid = (nextid+1)%MAX_STOCKORDER_ID;
	} while(idpool.find(nextid)!=idpool.end());
	idpool.insert(nextid);
	return nextid;
}
void StockExchange::FreeTid(unsigned int tid)
{
	idpool.erase(tid);
}
void StockExchange::HoldTid(unsigned int tid)
{
	idpool.insert(tid);
	if(tid>nextid)
		nextid = tid;
}
void StockExchange::Open() 
{ 
	AutoBalance();
	status = EXCHANGE_TRADING; 
	nextid += 86; // 尽量避免重启后用到重启前已完成的tid
	starttime = Timer::GetTime();
}

void StockExchange::AutoBalance()
{
	int price;
	int ack_volume, ack_money;
	if(balance>0)
		price = 1; // 有未匹配的买单，以最低价向市场卖出相应份额
	else
	{
		price = -MAX_STOCK_PRICE; // 有未匹配的卖单，以最高价从市场买入相应份额
		balance = -balance;
	}
	if(balance)
	{
		PriceMatching(0, price, balance, ack_volume, ack_money);
		Log::formatlog("autobalance","balance=%d:ack_volume=%d:ack_money=%d", balance, ack_volume, ack_money);
	}
	for(OrderMap::iterator it=orders.upper_bound(0);it!=orders.end();++it)
	{
		StockOrder& order = it->second;
		PriceMatching(0, order.price, order.volume, ack_volume, ack_money);
		if(!ack_volume)
			break;
		Log::formatlog("autobalance","tid=%d,ack_volume=%d,ack_money=%d", order.tid, ack_volume, ack_money);
		ReduceVolume(order.price, ack_volume);
		StockOrder o(order);
		o.volume = ack_volume;
		pending.push_back(PendingOrder(o, 0, ack_money));
		order.volume -= ack_volume;
		if(order.volume)
			break;
	}
}

void StockExchange::OnCommission(StockOrder& order, int ack_volume, int& cash, int& money, std::vector<StockPrice>& list)
{
	DEBUG_PRINT("StockExchange::OnCommission, tid=%d", order.tid);
	for(PendingList::iterator ip=pending.begin(),ie=pending.end();ip!=ie;++ip)
	{
		if(ip->dealtid!=order.tid)
			continue;
		if(ip->status==COMMISSION_WAIT)
			ip->status = COMMISSION_DONE;
	}

	int userid = order.userid;
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		it = accounts.insert(std::make_pair(userid, UserAccount(userid))).first;
	it->second.AddOrder(order);
	it->second.OnCommission(cash,money);

	order.volume -= ack_volume;
	if(order.volume)
	{
		AppendVolume(order.price, order.volume);
		orders.insert(std::make_pair(order.price, order));
		DEBUG_PRINT("Insert into orders::tid=%d, price=%d, volume=%d, size=%d", order.tid,order.price, 
			volumes[order.price], orders.size());
	}
	GetPrice(list);
}

void StockExchange::OnBalance(int userid,unsigned int tid,int ack_volume,int volume_left,int cash,int money)
{
	DEBUG_PRINT("OnBalance, userid=%d, tid=%d, ack_volume=%d, cash=%d, money=%d", userid, tid, ack_volume, cash, money);
	int price = 0;
	StockLog log;
	for(PendingList::iterator ip=pending.begin();ip!=pending.end();++ip)
	{
		if(ip->order.tid==tid && ip->status==BALANCE_WAIT)
		{
			log.tid = tid;
			log.time = Timer::GetTime();
			log.volume = ip->order.volume;
			log.cost = ip->ack_money;
			ip->status = BALANCE_DONE;
			price = ip->order.price;
			break;
		}
	}
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return;
	it->second.UpdateLog(log);
	if(it->second.OnBalance(tid,ack_volume,volume_left,cash,money))
	{
		DEBUG_PRINT("Order done, tid=%d, price=%d", tid, price);
		OrderMap::iterator io = FindOrder(tid, price);
		if(io!=orders.end())
			orders.erase(io);
		FreeTid(tid);
	}
}

void  StockExchange::PostCancel(int userid, unsigned int tid, short result, int cash, int money)
{
	LOG_TRACE("PostCancel, userid=%d, tid=%d, cash=%d, money=%d.",userid, tid, cash, money);
	AccountMap::iterator it = accounts.find(userid);
	if(it==accounts.end())
		return;
	int price;
	it->second.OnCancel(tid, result, price, cash, money);
	OrderMap::iterator io = FindOrder(tid, price);
	if(io!=orders.end())
		orders.erase(io);
	FreeTid(tid);
}

void StockExchange::AbortCommission(unsigned int tid)
{
	for(PendingList::iterator it=pending.begin();it!=pending.end();)
	{
		if(it->dealtid!=tid)
		{
			++it;
			continue;
		}

		if(it->order.tid!=tid)
		{
			OrderMap::iterator itmp = FindOrder(it->order.tid, it->order.price);
			if(itmp!=orders.end())
			{
				itmp->second.volume += it->order.volume;
				AppendVolume(it->order.price, it->order.volume);
			}
		}
		it = pending.erase(it);
	}
}

void StockExchange::OnDBConnect(Protocol::Manager *manager, int sid)
{
	if(status==EXCHANGE_INIT)
		manager->Send(sid,Rpc::Call(RPC_DBSTOCKLOAD,DBStockLoadArg()));
}

void StockExchange::ReduceVolume(int price, int volume)
{
	volumes[price] -= volume;
	if(volumes[price]==0)
		volumes.erase(price);
	order_volume -= volume;
}

void StockExchange::AppendVolume(int price, int volume)
{
	volumes[price] += volume;
	order_volume += volume;
}

bool StockExchange::OnLoad(std::vector<StockOrder>& list)
{
	for(std::vector<StockOrder>::iterator it=list.begin(),ie=list.end();it!=ie;++it)
	{
		if(it->tid==STOCK_BALANCE_ID)
			balance = it->volume;
		else if(it->volume>0 && it->price)
		{
			HoldTid(it->tid);
			AccountMap::iterator ia = accounts.find(it->userid);
			if(ia==accounts.end())
				ia = accounts.insert(std::make_pair(it->userid, UserAccount(it->userid))).first;
			ia->second.AddOrder(*it);
			AppendVolume(it->price, it->volume);
			orders.insert(std::make_pair(it->price, *it));
		}
		else
		{
			Log::log(LOG_ERR,"StockExchange:: invalid order,tid=%d,userid=%d,volume=%d", it->tid,it->userid,it->volume);
			return false;
		}
	}
	return true;
}

StockExchange::OrderMap::iterator StockExchange::FindOrder(unsigned int tid, int price)
{
	OrderMap::iterator ie = orders.end();
	for(OrderMap::iterator it=orders.lower_bound(price);it!=ie&&it->first==price;++it)
	{
		if(it->second.tid==tid)
			return it;
	}
	return ie;
}

void StockExchange::SetCashPassword(int userid, Octets& cash_password)
{
	AccountMap::iterator it = accounts.find(userid);
	if (it != accounts.end())
	{
		it->second.SetCashPassword(cash_password);
	}
	else
		Log::log(LOG_ERR, "set cash password can't found user, userid=%d", userid);
}

int StockExchange::CashUnlock(int userid, const Octets & cash_pwd)
{
	AccountMap::iterator it = accounts.find(userid);
	if (it == accounts.end())
		return ERR_STOCK_NOTFOUND;
	if (it->second.CashUnlock(cash_pwd))
		return ERR_SUCCESS;
	return ERR_STOCK_CASHUNLOCKFAILED;
}

int StockExchange::CashLock(int userid)
{
	AccountMap::iterator it = accounts.find(userid);
	if (it == accounts.end())
		return ERR_STOCK_NOTFOUND;
	it->second.CashLock();
	return ERR_SUCCESS;
}

bool StockExchange::IsCashLocked(int userid)
{
	AccountMap::iterator it = accounts.find(userid);
	if (it != accounts.end())
		return it->second.IsCashLocked();
	return false;
}
};

