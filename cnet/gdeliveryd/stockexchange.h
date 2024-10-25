#ifndef __GNET_STOCKEXCHANGE_H
#define __GNET_STOCKEXCHANGE_H

#include <vector>
#include <map>
#include <list>

#include "stocklog"
#include "stockprice"
#include "stockorder"
#include "localmacro.h"

namespace GNET
{

#define MAX_ACCOUNT_CASH   1000000    // 账户最大允许储蓄元宝
#define MAX_STOCKORDER_ID  1000000    // 最大订单ID
//在委托卖元宝时,实际所得金钱可能远高于委托价
//为防止在PriceMatching中ack_money越界，限制最高元宝单价*最高单比交易元宝数要小于40亿
#define MAX_STOCK_VOLUME   1000       // 最高单笔交易元宝数
#define MAX_STOCK_PRICE    4000000    // 最高元宝单价
#define MAX_ORDER_TIMEOUT  (86400*3)  // 订单超时时间

// 单价大于零表示出售元宝，单价绝对值应小于MAX_STOCK_PRICE且能被100整除

class UserAccount
{
public:
	int userid;
	int cash;
	int money;
	int cash_buy;
	int cash_sell;
	int status;
	time_t locktime;
	time_t passtime;
	std::vector<StockOrder> orders;
	std::vector<StockLog>   logs;
	Octets cash_password; 
	enum
	{
		FLG_PWDLOCKED = 0x00000001,
	};
	enum
	{
		CASH_UNLOCK = '0', CASH_LOCK = '1',
	};
	int flags;
	bool CheckFlag(int flag) { return (flag & flags) == flag;}
	void SetFlag(int flag) { flags |= flag; }
	void ClearFlag(int flag) { flags &= ~flag; }

	bool IsCashLocked() 
	{ 
		time_t now = Timer::GetTime();
		return cash_password.size() && (now-passtime>900 || CheckFlag(FLG_PWDLOCKED));
	}

	UserAccount(int id):userid(id),cash(0),money(0),cash_buy(0),cash_sell(0),status(0),locktime(0),flags(FLG_PWDLOCKED){ }
	void UpdateAccount(int _cash, int _money)
	{
		cash = _cash;
		money = _money;
	}
	void UpdateAccount(int _cash, int _money, int _buy, int _sell)
	{
		cash = _cash;
		money = _money;
		cash_buy = _buy;
		cash_sell = _sell;
	}
	void InitCashPassword(Octets & cash_pwd)
	{
		cash_password.swap(cash_pwd);
	}
	
	void SetCashPassword(Octets & cash_pwd)
	{
		cash_password.swap(cash_pwd);
		ClearFlag(FLG_PWDLOCKED);
	}
	bool Lock()
	{
		time_t now = Timer::GetTime();
		if(!status)
		{
			status = 1;
			locktime = now;
			return true;
		}
                if(now-locktime<1200)
			return false;
		Log::log(LOG_ERR,"StockAccount, lock timeout, userid=%d",userid);
		status = 1;
		locktime = now;
		return true;
	}
	bool UnLock()
	{
		if(!status)
			Log::log(LOG_ERR,"StockAccount, account is not locked, userid=%d",userid);
		status = 0;
		return true;
	}

	bool CashUnlock(const Octets& cash_pwd);
	void CashLock();
	void OnCommission(int& new_cash, int& new_money);
	bool OnBalance(unsigned int tid, int ack_volume, int volume, int new_cash, int new_money);
	bool OnCancel(unsigned int tid, short result, int& price, int new_cash, int new_money);
	bool OnTransaction(int retcode, int new_cash, int new_money);
	void UpdateLog(StockLog& log);
	int  OrderSize() { return orders.size(); }
	void AddOrder(const StockOrder& o) { orders.push_back(o); }
	void GetFreeFund(int& _cash, int& _money);
	void GetMaxFund(int& _cash, int& _money);
};

enum order_status{
	COMMISSION_WAIT, 
	COMMISSION_DONE,
	BALANCE_WAIT,
	BALANCE_DONE
};

class PendingOrder
{
public:
	StockOrder order;
	unsigned int dealtid;
	int ack_money;
	int status;
	PendingOrder(StockOrder& o,unsigned int tid, int m) : order(o),dealtid(tid),ack_money(m) 
	{ 
		status = tid ? COMMISSION_WAIT : COMMISSION_DONE;
	}
};

class StockExchange : public IntervalTimer::Observer
{
	typedef std::map<unsigned int,UserAccount>  AccountMap; // <userid, account>
	typedef std::multimap<int,StockOrder>  OrderMap;        // <price, order>
	typedef std::map<int,int>  VolumeMap;                   // <price, volume>
	typedef std::vector<PendingOrder>  PendingList;

	AccountMap     accounts;
	OrderMap       orders;
	VolumeMap      volumes;
	PendingList    pending;
	std::set<int>  idpool;

	int status;       
	unsigned int nextid;
	time_t checktime;
	time_t starttime;
	int balance;       
	int order_volume;      //当前挂单手数
	int trade_volume;      //累计成交手数
	int trade_amount;      //累计成交金额
	int trade_price;       //当前成交价格
public:
	enum{
		EXCHANGE_INIT,
		EXCHANGE_TRADING,
		EXCHANGE_HALTED
	};

	StockExchange() : status(EXCHANGE_INIT),nextid(0),checktime(0)
	{ 
		// TID(0, 1) 保留
		HoldTid(0);
		HoldTid(STOCK_BALANCE_ID);
		balance  = 0;
		order_volume = 0;
		trade_volume = 0;
		trade_price  = 0;
	}
	~StockExchange() { }
	static StockExchange* Instance() { static StockExchange instance; return &instance;}
	bool Initialize();
	void Open();
	void Close() { status = EXCHANGE_HALTED; }
	bool Update();
	void AutoBalance();
	bool LockAccount(int userid);

	bool OnLoad(std::vector<StockOrder>& list);
	void OnDBConnect(Protocol::Manager *manager, int sid);

	bool GetAccount(int userid, int& cash, int& money, char& locked, std::vector<StockPrice>& list);
	bool TradeLog(int userid, std::vector<StockOrder>& orders,  std::vector<StockLog>& logs);
	void GetPrice(std::vector<StockPrice>& orders);
	int  PlaceOrder(int userid, int price, int volume, int& ack_volume, unsigned int& tid);
	bool PriceMatching(unsigned int tid, int price, int volume, int& ack_volume, int& ack_money, bool dryrun=false);

	int  PreTransaction(int userid, char withdraw, int cash, int money, int pocket_cash, int pocket_money);
	void PostTransaction(int userid, int retcode, int& cash, int& money);
	int  PreCancel(int userid, unsigned int tid, int price);
	void PostCancel(int userid, unsigned int tid, short result, int cash, int money);
	void OnLogin(int userid, int cash, int money, int cash_buy, int cash_sell, std::vector<StockLog>& logs, Octets & cash_pwd);
	void OnLogout(int userid);
	void OnCommission(StockOrder& order, int ack_volume, int& cash, int&money, std::vector<StockPrice>& list);
	void OnBalance(int userid,unsigned int tid,int ack_volume,int volume_left,int new_cash,int new_money);
	void AbortCommission(unsigned int tid);
	void CheckTimeout();
	unsigned int ApplyTid();
	void FreeTid(unsigned int tid);
	void HoldTid(unsigned int tid);
	void ReduceVolume(int price, int volume);
	void AppendVolume(int price, int volume);
	void CancelOrder(StockOrder& order, int result);
	bool IsTrading(unsigned int tid);
	OrderMap::iterator FindOrder(unsigned int tid, int price);
	
	void SetCashPassword(int userid, Octets& cash_password);
	int CashUnlock(int userid, const Octets& cash_password);
	int CashLock(int userid);
	bool IsCashLocked(int userid);

};
};
#endif

