#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "timer.h"
#include "mapuser.h"
#include "maplinkserver.h"
#include "sysauctionmanager.h"
#include "sysauctionbid_re.hpp"
#include "sysauctionprepareitem.hrp"
#include "dbsysauctioncashtransfer.hrp"
#include "dbsysauctioncashspend.hrp"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"
#include "chatbroadcast.hpp"

namespace GNET
{

bool SysAuctionManager::Initialize()
{
	//从文件中读取竞拍列表
	FILE * f = fopen("sysauctionlist.txt", "r");
	if(!f)
	{
		Log::log(LOG_ERR, "SysAuctionManager::Initialize cannot open sysauctionlist.txt!");					
		return true;
	}
	std::map<std::string,int> conf_map;
	conf_map["config"] = 1;
	conf_map["items"] = 2;
	
	sell_item_list.clear();
	int section = 0;
	char buf[200];
	while(fgets(buf, 200, f))
	{
		char c = buf[0];
		if(c == '#' || c == ';') continue;
		if(c == '[')
		{
			std::string line(buf);
			string::size_type start = line.find_first_not_of(" \t", 1);
			if (start == string::npos) continue;
			string::size_type end   = line.find_first_of(" \t]", start);
			if (end   == string::npos) continue;
			std::string sec(line, start, end - start);
			
			section = conf_map[sec];	
			if(section == 0)
				Log::log(LOG_ERR, "unknown section: %s\n", sec.c_str());
		}
		else if(section == 1)
		{
			std::string line(buf);
			string::size_type key_start = line.find_first_not_of(" \t");
			if (key_start == string::npos) continue;
			string::size_type key_end   = line.find_first_of(" \t=", key_start);
			if (key_end == string::npos) continue;
			string::size_type val_start = line.find_first_of("=", key_end);
			if (val_start == string::npos) continue;
			val_start = line.find_first_not_of(" \t", val_start + 1);
			if (val_start == string::npos) continue;
			string::size_type val_end = line.find_last_not_of(" \t\r\n");
			if (val_end == string::npos) continue;
			if (val_end < val_start) continue;
			std::string key(line, key_start, key_end - key_start);
			std::string value(line, val_start, val_end - val_start + 1);

			if(strcmp(key.c_str(),"auction_max") == 0)
				auction_max = atoi(value.c_str());
			else
				Log::log(LOG_ERR, "unknown key: %s",key.c_str());
		}
		else if(section == 2)
		{
			int item_id, base_price, auction_time;
			sscanf(buf, "%d%d%d", &item_id, &base_price, &auction_time);
			if(item_id <= 0 || base_price <= 0 || auction_time < 600 || auction_time > 3600*3) 
			{
				Log::log(LOG_ERR, "sysauction initialize: sell item(item_id=%d base_price=%d auction_time=%d) illegal", item_id, base_price, auction_time);
				continue;	
			}
			sell_item_list.push_back(SellItem(item_id, base_price, auction_time));	
		}		
		else
		{
			Log::log(LOG_ERR, "section invalid: %d\n",section);
		}
	}
	fclose(f);

	if(auction_max < 0) auction_max = 0;
	if(auction_max > (int)sell_item_list.size()) auction_max = sell_item_list.size();
	if(auction_max > SYSAUCTION_MAX_COUNT) auction_max = SYSAUCTION_MAX_COUNT;
	
	LOG_TRACE("sysauction initialize: auction max=%d, total sell item=%d", auction_max, sell_item_list.size());
	
	IntervalTimer::Attach( this,SYSAUCTION_UPDATE_INTERVAL/IntervalTimer::Resolution() );	
	return true;
}

bool SysAuctionManager::Update()
{
	Thread::Mutex::Scoped l(lock);

	time_t t1 = GetTime();
	struct tm* tm1 = localtime(&t1);
	int second_of_day = tm1->tm_hour*3600 + tm1->tm_min*60 + tm1->tm_sec;
	
	if(second_of_day < SYSAUCTION_PREPAREITEM_TIME)
		;
	else if(second_of_day < SYSAUCTION_START_TIME)			//生成竞拍物品
	{
		if(!(status & ST_PREPARED))
			PrepareSellItem();	
		else if( --tick_counter <= 0)
		{
			tick_counter = 30;
			NotifyStartTime();
		}
	}
	else if(!sysauction_map.size())
		;
	else if(second_of_day < SYSAUCTION_END_TIME)			//竞拍开始
	{
		if(!(status & ST_START))
			StartSysAuction();
		else
			UpdateSysAuction();
	}
	else if(second_of_day < SYSAUCTION_CLEAR_TIME)			//强制所有竞拍结束
	{
		if(!(status & ST_END))
			EndSysAuction();
	}
	else 													//删除所有竞拍物品
	{
			ClearSellItem();
	}
	return true;
}

void SysAuctionManager::OnLogin(int userid, unsigned int cash, unsigned int cash_used)
{
	UserMap::iterator itu = user_map.find(userid);
	if(itu == user_map.end())
		itu = user_map.insert(std::make_pair(userid, UserSysAuctionInfo(userid))).first;
	itu->second.UpdateInfo(cash, cash_used);	
}

void SysAuctionManager::OnLogout(int userid)
{
	UserMap::iterator itu = user_map.find(userid);
	if(itu != user_map.end() && itu->second.BidSize() == 0)
		user_map.erase(itu);
}
	
void SysAuctionManager::PrepareSellItem()
{
	//竞拍物品已生成
	if(sysauction_map.size()) return;

	if(!auction_max) return;
	size_t sz = sell_item_list.size();	
	if(!sz) return;

	std::set<int> rset;
	int r = 0;
	while((int)rset.size() < auction_max)
	{
	 	r = rand()%sz;
		rset.insert(r);
	}
	
	SysAuctionPrepareItemArg arg;
	std::set<int>::iterator it = rset.begin(), ite = rset.end();
	for(; it!=ite; ++it)
	{
		arg.indexes.push_back(*it);
		arg.item_ids.push_back(sell_item_list[*it].item_id);
	}
	SysAuctionPrepareItem * rpc = (SysAuctionPrepareItem *)Rpc::Call(RPC_SYSAUCTIONPREPAREITEM,arg);
	GProviderServer::GetInstance()->DispatchProtocol(1,rpc);	//发给gs01	
}

void SysAuctionManager::NotifyStartTime()
{
	//广播
	ChatBroadCast cbc;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = CMSG_SYSAUCTION_FORENOTICE;
	Marshal::OctetsStream os;
	os << (int)SYSAUCTION_START_TIME;
	cbc.msg = os;
	LinkServer::GetInstance().BroadcastProtocol(cbc);
}

void SysAuctionManager::StartSysAuction()
{
	SysAuctionMap::iterator it = sysauction_map.begin(), ie = sysauction_map.end();
	for(; it!=ie; ++it)
	{
		GSysAuctionDetail & detail = it->second.GetDetail();
		if(detail.info.state == SysAuctionObj::STATE_PREPARE_START)
		{
			detail.info.state = SysAuctionObj::STATE_START;	
			//广播物品
			ChatBroadCast cbc;
			cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
			cbc.srcroleid = CMSG_SYSAUCTION_START;
			Marshal::OctetsStream os;
			os << detail.info.sa_id << detail.info.item_id << detail.info.base_price;
			cbc.msg = os;
			LinkServer::GetInstance().BroadcastProtocol(cbc);
		}
	}
	status |= ST_START;
}

void SysAuctionManager::UpdateSysAuction()
{
	int now = GetTime();
	SysAuctionMap::iterator it = sysauction_map.begin(), ie = sysauction_map.end();
	for(; it!=ie; ++it)
	{
		if(it->second.IsBusy()) continue;
		GSysAuctionDetail & detail = it->second.GetDetail();
		if(detail.info.state == SysAuctionObj::STATE_START)
		{
			if(now >= detail.info.auction_endtime || 
					detail.info.bid_freezetime && now >= detail.info.bid_freezetime)
			{
				if(detail.info.bidder_userid)
				{
					detail.info.state = SysAuctionObj::STATE_PREPARE_END;
					//广播
					if(now >= detail.info.bid_freezetime)
					{
						ChatBroadCast cbc;
						cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
						cbc.srcroleid = CMSG_SYSAUCTION_BIDINFO;
						Marshal::OctetsStream os;
						os << detail.info.sa_id << detail.info.item_id << detail.info.bid_price << detail.info.bidder_roleid << (int)SYSAUCTION_BID_FREEZETIME << (int)0;
						cbc.msg = os;
						LinkServer::GetInstance().BroadcastProtocol(cbc);
					}
					else
					{
						ChatBroadCast cbc;
						cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
						cbc.srcroleid = CMSG_SYSAUCTION_END;
						Marshal::OctetsStream os;
						os << detail.info.sa_id << detail.info.item_id << detail.info.bid_price << detail.info.bidder_roleid;
						cbc.msg = os;
						LinkServer::GetInstance().BroadcastProtocol(cbc);
					}
				}
				else
					detail.info.state = SysAuctionObj::STATE_END;
				detail.info.auction_endtime = now;
			}
			else if(detail.info.bidder_userid)
			{
				int left_time = detail.info.bid_freezetime-now;
				int tick_time = SYSAUCTION_UPDATE_INTERVAL/1000000;
				if((left_time/tick_time)%(180/tick_time) == 0)
				{
					//广播
					ChatBroadCast cbc;
					cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
					cbc.srcroleid = CMSG_SYSAUCTION_BIDINFO;
					Marshal::OctetsStream os;
					os << detail.info.sa_id << detail.info.item_id << detail.info.bid_price << detail.info.bidder_roleid << (int)SYSAUCTION_BID_FREEZETIME << left_time;
					cbc.msg = os;
					LinkServer::GetInstance().BroadcastProtocol(cbc);
				}
			}
		}
		if(detail.info.state == SysAuctionObj::STATE_PREPARE_END)
		{
			it->second.SetBusy(true); 
			//发放	
			UserMap::iterator itu = user_map.find(detail.info.bidder_userid);
			if(itu == user_map.end())
			{
				Log::log(LOG_ERR,"SysAuctionManager::UpdateSysAuction user(%d) cannot found", detail.info.bidder_userid);
				continue;
			}
			if(!itu->second.VerifyBid(detail.info.sa_id,detail.info.bid_price))
			{
				Log::log(LOG_ERR,"SysAuctionManager::UpdateSysAuction verify bid failed. user=%d,sa_id=%d,bid_price=%d", detail.info.bidder_userid, detail.info.sa_id, detail.info.bid_price);
				continue;
			}
		
			itu->second.SetBusy(true);
			DBSysAuctionCashSpend * rpc = (DBSysAuctionCashSpend *)Rpc::Call(
					RPC_DBSYSAUCTIONCASHSPEND,
					DBSysAuctionCashSpendArg(
						detail.info.bidder_userid,
						detail.info.bidder_roleid,
						detail.info.bid_price,
						detail.info.sa_id,
						detail.item)
					);		
			GameDBClient::GetInstance()->SendProtocol(rpc);	
		}
	}
}

void SysAuctionManager::EndSysAuction()
{
	size_t end_cnt = 0;
	SysAuctionMap::iterator it = sysauction_map.begin(), ie = sysauction_map.end();
	for(; it!=ie; ++it)
	{
		if(it->second.IsBusy()) continue;
		GSysAuctionDetail & detail = it->second.GetDetail();
		if(detail.info.state == SysAuctionObj::STATE_START)
		{
			//强制结束
			if(detail.info.bidder_userid)
			{
				detail.info.state = SysAuctionObj::STATE_PREPARE_END;
				//广播
				ChatBroadCast cbc;
				cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
				cbc.srcroleid = CMSG_SYSAUCTION_END;
				Marshal::OctetsStream os;
				os << detail.info.sa_id << detail.info.item_id << detail.info.bid_price << detail.info.bidder_roleid;
				cbc.msg = os;
				LinkServer::GetInstance().BroadcastProtocol(cbc);
			}
			else
				detail.info.state = SysAuctionObj::STATE_END;
			detail.info.auction_endtime = GetTime();
		}
		if(detail.info.state == SysAuctionObj::STATE_PREPARE_END)
		{
			it->second.SetBusy(true); 
			//发放	
			UserMap::iterator itu = user_map.find(detail.info.bidder_userid);
			if(itu == user_map.end())
			{
				Log::log(LOG_ERR,"SysAuctionManager::EndSysAuction user(%d) cannot found. roleid=%d", detail.info.bidder_userid, detail.info.bidder_roleid);
				continue;
			}
			if(!itu->second.VerifyBid(detail.info.sa_id,detail.info.bid_price))
			{
				Log::log(LOG_ERR,"SysAuctionManager::EndSysAuction verify bid failed. userid=%d,roleid=%d,sa_id=%u,bid_price=%u", detail.info.bidder_userid, detail.info.bidder_roleid, detail.info.sa_id, detail.info.bid_price);
				continue;
			}
		
			itu->second.SetBusy(true);
			DBSysAuctionCashSpend * rpc = (DBSysAuctionCashSpend *)Rpc::Call(
					RPC_DBSYSAUCTIONCASHSPEND,
					DBSysAuctionCashSpendArg(
						detail.info.bidder_userid,
						detail.info.bidder_roleid,
						detail.info.bid_price,
						detail.info.sa_id,
						detail.item)
					);		
			GameDBClient::GetInstance()->SendProtocol(rpc);	
		}
		if(detail.info.state == SysAuctionObj::STATE_END)
			end_cnt ++;
	}
	if(end_cnt == sysauction_map.size())
		status |= ST_END;
}

void SysAuctionManager::ClearSellItem()
{
	SysAuctionMap::iterator it = sysauction_map.begin(), ie = sysauction_map.end();
	for(; it!=ie; ++it)
	{
		GSysAuctionDetail & detail = it->second.GetDetail();
		if(detail.info.state != SysAuctionObj::STATE_END)
		{
			Log::log(LOG_ERR,"SysAuctionManager::ClearSellItem wrong state. sa_id=%u,item_id=%d,item_count=%d,state=%d,busy=%d,userid=%d,roleid=%d,bid_price=%u", detail.info.sa_id, detail.info.item_id, detail.info.item_count, detail.info.state, it->second.IsBusy(), detail.info.bidder_userid, detail.info.bidder_roleid, detail.info.bid_price);
			if(detail.info.bidder_userid)
			{
				UserMap::iterator itu = user_map.find(detail.info.bidder_userid);
				if(itu != user_map.end())
					itu->second.DelBid(detail.info.sa_id);			
			}
		}
	}
	sysauction_map.clear();
	status = 0;
	max_id = GetTime() + 3600;
	tick_counter = 0;
}

void SysAuctionManager::GetSysAuctionList(std::vector<GSysAuctionItem>& items)
{
	Thread::Mutex::Scoped l(lock);
	
	items.clear();
	SysAuctionMap::iterator it = sysauction_map.begin(), ie = sysauction_map.end();
	for(; it!=ie; ++it)
		items.push_back(it->second.GetDetail().info);
}

bool SysAuctionManager::GetSysAuction(unsigned int sa_id, GSysAuctionDetail & detail)
{
	Thread::Mutex::Scoped l(lock);
	
	SysAuctionMap::iterator it = sysauction_map.find(sa_id);
	if(it == sysauction_map.end()) return false;
	detail = it->second.GetDetail();	
	return true;
}

int SysAuctionManager::GetSysAuctionAccount(int userid, unsigned int & cash, std::vector<unsigned int>& bid_ids)
{
	Thread::Mutex::Scoped l(lock);
	
	UserMap::iterator itu = user_map.find(userid);	
	if(itu == user_map.end()) return ERR_SA_USER_NOT_FOUND;
	cash = itu->second.GetFreeCash();
	itu->second.GetBid(bid_ids);
	return ERR_SUCCESS;
}

void SysAuctionManager::OnGSPrepareItem(std::vector<int>& indexes, GRoleInventoryVector & items)
{
	Thread::Mutex::Scoped l(lock);
	//竞拍物品已生成
	if(sysauction_map.size()) return;
	//计算拍卖起始时间
	time_t now = GetTime();
	struct tm * tm1 = localtime(&now);
	int auction_starttime = now - tm1->tm_hour*3600 - tm1->tm_min*60 - tm1->tm_sec + SYSAUCTION_START_TIME;
	
	for(size_t i=0; i<indexes.size()&&i<items.size(); i++)
	{
		int m = indexes[i];
		if(m >= (int)sell_item_list.size())	continue;
		if(sell_item_list[m].item_id != (int)items[i].id) continue;
		GSysAuctionDetail detail;
		detail.info.sa_id 		= ApplyID();
		detail.info.item_id 	= items[i].id;
		detail.info.item_count 	= 1;
		detail.info.base_price 	= sell_item_list[m].base_price;
		detail.info.state 		= SysAuctionObj::STATE_PREPARE_START;
		detail.info.auction_starttime = auction_starttime;
		detail.info.auction_endtime	= auction_starttime + sell_item_list[m].auction_time;
		items[i].count	= 1;
		detail.item 	= items[i];
		sysauction_map.insert(std::make_pair(detail.info.sa_id,SysAuctionObj(detail)));
	}
	if(sysauction_map.size())
	{
		status |= ST_PREPARED;
		tick_counter = 1;
	}
}

int SysAuctionManager::TryBid(int userid, int roleid, unsigned int sa_id, unsigned int bid_price, unsigned int& cash, GSysAuctionItem & info)
{
	Thread::Mutex::Scoped l(lock);

	UserMap::iterator itu = user_map.find(userid);
	if(itu == user_map.end()) return ERR_SA_USER_NOT_FOUND;
	if(itu->second.IsBusy()) return ERR_SA_USER_IS_BUSY;
	if(bid_price > itu->second.GetFreeCash()) return ERR_SA_CASH_NOT_ENOUGH;
	
	SysAuctionMap::iterator it = sysauction_map.find(sa_id);
	if(it == sysauction_map.end()) return ERR_SA_ENTRY_NOT_FOUND;
	if(it->second.IsBusy()) return ERR_SA_ENTRY_IS_BUSY;
	GSysAuctionDetail & detail = it->second.GetDetail();
	if(detail.info.state != SysAuctionObj::STATE_START) return ERR_SA_CANNOT_BID;
	if(detail.info.bidder_userid == userid) return ERR_SA_CANNOT_BID;
	if(bid_price%100 != 0 || bid_price < detail.info.base_price) return ERR_SA_LOW_BIDPRICE;
	if(detail.info.bid_price && bid_price < detail.info.bid_price+100) return ERR_SA_LOW_BIDPRICE;
	
	//先保存失败者
	unsigned int prev_price = detail.info.bid_price;
	int prev_role = detail.info.bidder_roleid;
	int prev_user = detail.info.bidder_userid;

	detail.info.bid_price = bid_price;
	detail.info.bid_time = GetTime();
	detail.info.bid_freezetime = detail.info.bid_time + SYSAUCTION_BID_FREEZETIME;
	detail.info.bidder_roleid = roleid;
	detail.info.bidder_userid = userid;
	itu->second.AddBid(detail.info.sa_id, bid_price);
	//延长拍卖时间
	if(detail.info.bid_time > detail.info.auction_endtime - 60)
		detail.info.auction_endtime += 60;
	//广播
	ChatBroadCast cbc;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = CMSG_SYSAUCTION_BID;
	Marshal::OctetsStream os;
	os << detail.info.sa_id << detail.info.item_id << detail.info.bid_price << detail.info.bidder_roleid;
	cbc.msg = os;
	LinkServer::GetInstance().BroadcastProtocol(cbc);
	
	cash = itu->second.GetFreeCash();
	info = detail.info;
	//处理失败者
	if(prev_user)
	{
		UserMap::iterator itu2 = user_map.find(prev_user);
		if(itu2 == user_map.end()) 
		{
			Log::log(LOG_ERR, "SysAuctionManager::TryBid prev bidder(%d) cannot found. roleid=%d", prev_user, prev_role);
		}
		else if(!itu2->second.VerifyBid(detail.info.sa_id,prev_price))
		{
			Log::log(LOG_ERR,"SysAuctionManager::TryBid verify bid failed. prev bidder=%d,roleid=%d,sa_id=%u,bid_price=%u", prev_user, prev_role, detail.info.sa_id, prev_price);
		}
		else
		{
			itu2->second.DelBid(detail.info.sa_id);
			//试图通知一下失败者
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(prev_role);
			if ( NULL!=pinfo )
			{
				SysAuctionBid_Re re;
				re.retcode = ERR_SA_BID_FAILED; 
				re.cash = itu2->second.GetFreeCash();
				re.info = detail.info;
				re.localsid = pinfo->localsid;
				GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
			}
		}	
	}
	return ERR_SUCCESS;
}

int SysAuctionManager::TryCashTransfer(char withdraw, unsigned int cash, PlayerInfo& ui, GMailSyncData& sync)
{
	Thread::Mutex::Scoped l(lock);

	UserMap::iterator itu = user_map.find(ui.userid);
	if(itu == user_map.end()) return ERR_SA_USER_NOT_FOUND;
	if(itu->second.IsBusy()) return ERR_SA_USER_IS_BUSY;
	unsigned int pocket_cash = sync.cash_total-sync.cash_used;
	unsigned int free_cash = itu->second.GetFreeCash();
	unsigned int max_cash = itu->second.GetMaxCash();
	if(withdraw)
	{
		if(cash > free_cash) return ERR_SA_CASH_NOT_ENOUGH_FOR_TRANSFER; 
	}
	else
	{
		if(cash > pocket_cash) return ERR_SA_CASH_NOT_ENOUGH_FOR_TRANSFER;
		if(cash > SYSAUCTION_MAX_CASH - max_cash) return ERR_SA_CASH_OVERFLOW;
	}

	itu->second.SetBusy(true);
	DBSysAuctionCashTransfer * rpc = (DBSysAuctionCashTransfer *)Rpc::Call(
			RPC_DBSYSAUCTIONCASHTRANSFER,
			DBSysAuctionCashTransferArg(
				ui.userid,
				ui.roleid,
				withdraw,
				cash,
				sync)
			);	
	rpc->save_linksid = ui.linksid;
	rpc->save_localsid = ui.localsid;
	rpc->save_gsid = ui.gameid;
	GameDBClient::GetInstance()->SendProtocol(rpc);

	return ERR_SUCCESS;
}

bool SysAuctionManager::OnDBCashTranfer(int userid, unsigned int& cash)
{
	Thread::Mutex::Scoped l(lock);

	UserMap::iterator itu = user_map.find(userid);
	if(itu == user_map.end()) return false;
	itu->second.UpdateInfo(cash);
	cash = itu->second.GetFreeCash();
	return true;
}

bool SysAuctionManager::OnDBCashSpend(unsigned int sa_id, int userid, unsigned int cash, unsigned int cash_used)
{
	Thread::Mutex::Scoped l(lock);

	SysAuctionMap::iterator it = sysauction_map.find(sa_id);
	if(it != sysauction_map.end())
	{
		GSysAuctionDetail & detail = it->second.GetDetail();
		if(detail.info.state == SysAuctionObj::STATE_PREPARE_END)
			detail.info.state = SysAuctionObj::STATE_END;
	}
	
	UserMap::iterator itu = user_map.find(userid);
	if(itu != user_map.end())
	{
		itu->second.DelBid(sa_id);
		itu->second.UpdateInfo(cash,cash_used);
	}
	return true;
}

void SysAuctionManager::ClearUserBusy(int userid)
{
	Thread::Mutex::Scoped l(lock);

	UserMap::iterator itu = user_map.find(userid);
	if(itu == user_map.end()) return;
	itu->second.SetBusy(false);
}

void SysAuctionManager::ClearSysAuctionBusy(unsigned int sa_id)
{
	Thread::Mutex::Scoped l(lock);

	SysAuctionMap::iterator it = sysauction_map.find(sa_id);
	if(it == sysauction_map.end()) return;
	it->second.SetBusy(false);
}

int SysAuctionManager::GetTime()
{
	return Timer::GetTime() + adjust_time; 
}

void SysAuctionManager::SetAdjustTime(int t)
{
	if(t == 0)
		adjust_time = 0;
	else if(t > 0) 
		adjust_time += t;
	DEBUG_PRINT("SysAuctionManager::SetAdjustTime t=%d", t);
}

}
