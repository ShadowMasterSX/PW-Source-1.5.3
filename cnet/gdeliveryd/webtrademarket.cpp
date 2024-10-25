#include "gmailsyncdata"
#include "gwebtraderolebrief"
#include "webtradeprepost.hpp"
#include "webtraderoleprepost.hpp"
#include "dbwebtradeload.hrp"
#include "dbwebtradeloadsold.hrp"
#include "dbwebtradeprepost.hrp"
#include "dbwebtradeprecancelpost.hrp"
#include "dbwebtradepost.hrp"
#include "dbwebtradecancelpost.hrp"
#include "dbwebtradeshelf.hrp"
#include "dbwebtradecancelshelf.hrp"
#include "dbwebtradesold.hrp"
#include "dbwebtradepostexpire.hrp"
#include "post.hpp"
#include "gamepostcancel.hpp"
#include "gdeliveryserver.hpp"
#include "gwebtradeclient.hpp"
#include "newkeepalive.hpp"
#include "mapuser.h"
#include "mapforbid.h"
#include "webtrademarket.h"

namespace GNET
{

bool WebTradeMarket::Initialize()
{
	if(!LoadConfig())
	{
		Log::log(LOG_ERR, "WebTradeMarket load Config failed!");					
		return false;
	}
	IntervalTimer::Attach( this,WEBTRADE_UPDATE_INTERVAL/IntervalTimer::Resolution() );	
	return true;
}

bool WebTradeMarket::LoadConfig()
{
	//从conf中读取游戏币寄售下限、公示期时长、寄售时长、保证金数量等
	Conf * conf = Conf::GetInstance();
	min_post_money = atoi(conf->find("GWebTradeClient","min_post_money").c_str());
	show_period = atoi(conf->find("GWebTradeClient","show_period").c_str());
	post_period = atoi(conf->find("GWebTradeClient","post_period").c_str());
	post_deposit = atoi(conf->find("GWebTradeClient","post_deposit").c_str());
	if(min_post_money < DEFAULT_MIN_POST_MONEY) min_post_money = DEFAULT_MIN_POST_MONEY;
	if(show_period <= 0) show_period = DEFAULT_SHOW_PERIOD;
	if(post_period <= 0) post_period = DEFAULT_POST_PERIOD;
	if(post_deposit <= 0) post_deposit = DEFAULT_POST_DEPOSIT;
	
	zoneid = atoi(conf->find("GDeliveryServer", "zoneid").c_str());
	aid = atoi(conf->find("GDeliveryServer", "aid").c_str());
	if(zoneid <= 0 || aid <= 0) return false;
	sn_man.Initialize(zoneid);	

	//从文件中读取物品分类
	char buf[200];
	FILE *f = fopen("webtradeid.txt", "r");
	if(!f)
		return true;
	classify_map.clear();
	while( fgets(buf, 200, f) )
	{
		unsigned int idItem, idCategory;
		sscanf(buf, "%u%u", &idItem, &idCategory);
		classify_map[idItem] = idCategory;
	}
	fclose(f);
	Log::formatlog("webtrade","initialize: total category=%d", classify_map.size()); 
	return true;
}

void WebTradeMarket::OnDBConnect(Protocol::Manager * manager, int sid)
{
	if(status == ST_INIT)
		manager->Send(sid,Rpc::Call(RPC_DBWEBTRADELOAD,DBWebTradeLoadArg()));
}

void WebTradeMarket::OnDBLoad(int64_t max_sn, std::vector<GWebTradeDetail>& list, bool finish)
{
	Thread::Mutex::Scoped l(lock);
	if(status != ST_INIT) return;
	
	if(max_sn > 0) sn_man.SetMaxSN(max_sn);

	for(size_t i=0; i<list.size(); i++)
	{
		GWebTradeDetail & item = list[i];
		category_t category = GetCategory(item.info.posttype, item.info.item_id);
		if(!category)
		{
			Log::formatlog("webtrade","put unknown item(posttype%d id%d) to category 1.", item.info.posttype, item.info.item_id);
			category = 1;
		}
		sn_map[item.info.sn] = category_map.insert(std::make_pair(category,WebTradeObj(item)));
		role_sell_map[item.info.seller_roleid].insert(item.info.sn);
		if(item.info.buyer_roleid > 0)
			role_buy_map[item.info.buyer_roleid].insert(item.info.sn);
		sn_man.HoldSN(item.info.sn);	
	}
	
	if(finish)
	{
		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBWEBTRADELOADSOLD,DBWebTradeLoadSoldArg()));	
	}	
}

void WebTradeMarket::OnDBLoadSold(std::vector<int64_t>& list, bool finish)
{
	Thread::Mutex::Scoped l(lock);
	if(status != ST_INIT) return;
	
	for(size_t i=0; i<list.size(); i++)
	{
		sold_set.insert(list[i]);
		sn_man.HoldSN(list[i]);	
	}	

	if(finish)
	{
		Log::formatlog("webtrade","initwebtrade: total=%d maprole=%d(sell)+%d(buy) sold_set.size=%d", category_map.size(), role_sell_map.size(), role_buy_map.size(), sold_set.size()); 
		status = ST_OPEN;	
	}
}

bool WebTradeMarket::Update()
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen()) return true;

	if(++tick >= 300)
	{
		tick = 0;
		GWebTradeClient::GetInstance()->SendProtocol(NewKeepAlive(0));
	}
	
	struct timeval now;
	IntervalTimer::GetTime(&now);
	CategoryMap::iterator it = category_map.begin(),ite = category_map.end();
	for( ; it!=ite; ++it)
	{
		WebTradeObj & obj = it->second;
		if(obj.IsBusy() && !obj.Update(WEBTRADE_UPDATE_INTERVAL))
		{
			continue;	
		}
	
		GWebTradeDetail & detail = obj.GetDetail();
		if(detail.info.state == WebTradeObj::STATE_PRE_POST)
		{
			SendPost(it->first, obj);	
		}
		else if(detail.info.state == WebTradeObj::STATE_PRE_CANCEL_POST)
		{
			SendCancelPost(obj);	
		}
		else
		{
			if(detail.info.state == WebTradeObj::STATE_SHOW)
			{
				if(now.tv_sec >= detail.info.show_endtime)
				{
					detail.info.state = WebTradeObj::STATE_SELL;
					detail.info.show_endtime = 0;
					//对于这个操作数据库不更新问题不大
				}			
			}
			if(detail.info.state == WebTradeObj::STATE_SELL)
			{
				if(now.tv_sec >= detail.info.sell_endtime)
				{
					if(detail.info.buyer_roleid > 0)
						role_buy_map[detail.info.buyer_roleid].erase(detail.info.sn);
					detail.info.state = WebTradeObj::STATE_POST;	
					detail.info.price = 0;
					detail.info.sell_endtime = 0;
					detail.info.buyer_roleid = 0;
					detail.info.buyer_userid = 0;
					detail.info.buyer_name.clear();
					//对于这个操作数据库不更新问题不大
				}			
			}
		}
	}

	return true;	
}

WebTradeMarket::category_t WebTradeMarket::GetCategory(int posttype, int item_id)
{
	if(posttype == 1)
		item_id = MONEY_MATTER_ID;
	else if(posttype == 2)
		;
	else if(posttype == 4)
		item_id = ROLE_DUMMY_ID;
	else
		return 0;
	ClassifyMap::iterator it = classify_map.find(item_id);
	if(it != classify_map.end())
		return it->second;
	else
		return 0;
}

bool WebTradeMarket::RemoveItem(int roleid, int64_t sn, bool freesn)
{
	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return false;

	if(freesn) sn_man.FreeSN(detail.info.sn);
	role_sell_map[detail.info.seller_roleid].erase(detail.info.sn);
	if(detail.info.buyer_roleid)
		role_buy_map[detail.info.buyer_roleid].erase(detail.info.sn);
	category_map.erase(it->second);
	sn_map.erase(it);
	return true;
}

int WebTradeMarket::GetAttendListNum(int roleid, bool getsell)
{
	RoleMap::iterator it, ie;
	if(getsell)
	{
		it = role_sell_map.find(roleid);	
		ie = role_sell_map.end();
	}
	else
	{
		it = role_buy_map.find(roleid);
		ie = role_buy_map.end();
	}
	return (it!=ie ? it->second.size() : 0);
}

void WebTradeMarket::ClearBusy(int64_t sn)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen()) return;		

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return;
	WebTradeObj & obj = it->second->second;
	if(obj.IsBusy())
		obj.SetBusy(false);	
	else
		Log::log(LOG_ERR, "WebTradeMarket error occur in ClearBusy()! sn(%lld) roleid(%d) state(%d)",
			obj.GetDetail().info.sn,obj.GetDetail().info.seller_roleid,obj.GetDetail().info.state);					
}

int WebTradeMarket::TryPrePost(WebTradePrePost& proto, PlayerInfo& ui, int ip, GMailSyncData & sync)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;
	//检查封禁...
	
	if(GetAttendListNum(proto.roleid) >= MAX_ATTEND_SELL) return ERR_WT_TOO_MANY_ATTEND_SELL;
	if(PostOffice::GetInstance().GetMailBoxSize(proto.roleid) >= SYS_MAIL_LIMIT) return ERR_WT_MAILBOX_FULL;
	//检查是否在可寄售列表,物品本身是否可交易在gamedbd检查
	category_t c = GetCategory(proto.posttype,proto.item_id);
	if(!c) return ERR_WT_CANNOT_POST; 
	//检查保证金
	unsigned int deposit = DEFAULT_POST_DEPOSIT; 
	switch(proto.posttype)
	{
		case 1:		//寄售游戏币
			if(proto.money < DEFAULT_MIN_POST_MONEY) return ERR_WT_CANNOT_POST;	
			if(deposit > sync.inventory.money - proto.money) return ERR_WT_NOT_ENOUGH_DEPOSIT;	
			proto.item_id = 0;
			proto.item_pos = 0;
			proto.item_num = 0;
		break;		
		case 2:		//寄售物品
			if(proto.item_num <= 0) return ERR_WT_CANNOT_POST;
			if(deposit > sync.inventory.money) return ERR_WT_NOT_ENOUGH_DEPOSIT;
			proto.money = 0;
		break;
		default:
			return ERR_WT_CANNOT_POST;
		break;
	}
	if(proto.price > 0)	//寄售同时要上架,proto.buyer_roleid是否有效在gamedbd检查	
	{
		if(proto.price > 100000000 || proto.price < 1000) return ERR_WT_ILLEGAL_SELL_PRICE; 
		if(proto.sellperiod%(24*3600) != 0 
			|| proto.sellperiod <= 0 || proto.sellperiod > MAX_SELL_PERIOD) return ERR_WT_ILLEGAL_SELL_PERIOD;	
		if(proto.buyer_roleid == proto.roleid) return ERR_WT_CANNOT_POST;
	}
	else
	{
		proto.price = 0;
		proto.sellperiod = 0;
		proto.buyer_roleid = 0;
	}
	int64_t sn = ApplySN();
	if(sn == 0)	return ERR_WT_SN_EXHAUSE;
	//query DB	
	struct timeval now;
	IntervalTimer::GetTime(&now);
	int64_t post_time = now.tv_sec * (int64_t)1000 + now.tv_usec/1000;
	int state = WebTradeObj::STATE_PRE_POST;
	DBWebTradePrePost* rpc=(DBWebTradePrePost*)Rpc::Call( 
			RPC_DBWEBTRADEPREPOST,
			DBWebTradePrePostArg(
				sn,
				proto.roleid,
				ui.userid,
				ui.name,
				proto.posttype,
				proto.money,
				proto.item_id,
				proto.item_pos,
				proto.item_num,
				proto.price,
				proto.sellperiod,
				proto.buyer_roleid,
				post_time,
				state,
				deposit,
				ip,
				sync
			)
		);
	rpc->save_linksid=ui.linksid;
	rpc->save_localsid=ui.localsid;
	rpc->save_gsid=ui.gameid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool WebTradeMarket::OnDBPrePost(const GWebTradeDetail & detail)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;
	
	category_t c = GetCategory(detail.info.posttype, detail.info.item_id);
	if(!c) c = 1;
	CategoryMap::iterator it = category_map.insert(std::make_pair(c,WebTradeObj(detail)));
	sn_map[detail.info.sn] = it;
	role_sell_map[detail.info.seller_roleid].insert(detail.info.sn);
	if(detail.info.buyer_roleid)
		role_buy_map[detail.info.buyer_roleid].insert(detail.info.sn);

	SendPost(it->first, it->second);
	return true;
}

int WebTradeMarket::TryPreCancelPost(int roleid, int64_t sn, PlayerInfo& ui)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;

	if(PostOffice::GetInstance().GetMailBoxSize(roleid) >= SYS_MAIL_LIMIT) return ERR_WT_MAILBOX_FULL;
	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	if(it->second->second.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(detail.info.state != WebTradeObj::STATE_POST) return ERR_WT_CANNOT_CANCELPOST;
	if(detail.info.posttype != 1 && detail.info.posttype != 2) return ERR_WT_CANNOT_CANCELPOST;

	it->second->second.SetBusy(true);
	//query DB
	int state = WebTradeObj::STATE_PRE_CANCEL_POST;
	DBWebTradePreCancelPost * rpc = (DBWebTradePreCancelPost *)Rpc::Call(
			RPC_DBWEBTRADEPRECANCELPOST,
			DBWebTradePreCancelPostArg(
				sn,
				roleid,
				state
			)
		);	
	rpc->save_linksid=ui.linksid;
	rpc->save_localsid=ui.localsid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;	
}

bool WebTradeMarket::OnDBPreCancelPost(int roleid, int64_t sn)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return false;

	detail.info.state = WebTradeObj::STATE_PRE_CANCEL_POST;
	
	SendCancelPost(it->second->second);
	return true;
}

int WebTradeMarket::TryRolePrePost(WebTradeRolePrePost& proto, const UserInfo& ui)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;
	//同一账号下只能寄售一个角色
	for(int i=0; i<16; i++)
		if(ui.role_status[i] == _ROLE_STATUS_FROZEN)
			return ERR_WT_USER_OTHER_ROLE_ON_SALE;
	//检查封禁
	GRoleForbid	forbid;
	if(ForbidRoleLogin::GetInstance().GetForbidRoleLogin(proto.roleid, forbid)) return ERR_ROLEFORBID;
	//不能有其他寄售品存在
	if(GetAttendListNum(proto.roleid) >= 1) return ERR_WT_HAS_ATTEND_SELL;
	if(PostOffice::GetInstance().GetMailBoxSize(proto.roleid) >= SYS_MAIL_LIMIT) return ERR_WT_MAILBOX_FULL;
	//检查是否在可寄售列表,角色本身是否可交易在gamedbd检查
	category_t c = GetCategory(4/*角色*/,0);
	if(!c) return ERR_WT_CANNOT_POST; 
	//检查保证金在gamedbd
	unsigned int deposit = DEFAULT_POST_DEPOSIT; 
	if(proto.price > 0)	//寄售同时要上架,proto.buyer_roleid是否有效在gamedbd检查	
	{
		if(proto.price > 100000000 || proto.price < 6000) return ERR_WT_ILLEGAL_SELL_PRICE; 
		if(proto.sellperiod%(24*3600) != 0 
			|| proto.sellperiod <= 0 || proto.sellperiod > MAX_SELL_PERIOD) return ERR_WT_ILLEGAL_SELL_PERIOD;	
		if(proto.buyer_roleid == proto.roleid) return ERR_WT_CANNOT_POST;
	}
	else
	{
		proto.price = 0;
		proto.sellperiod = 0;
		proto.buyer_roleid = 0;
	}
	int64_t sn = ApplySN();
	if(sn == 0)	return ERR_WT_SN_EXHAUSE;
	//query DB	
	struct timeval now;
	IntervalTimer::GetTime(&now);
	int64_t post_time = now.tv_sec * (int64_t)1000 + now.tv_usec/1000;
	int state = WebTradeObj::STATE_PRE_POST;
	DBWebTradePrePost* rpc=(DBWebTradePrePost*)Rpc::Call( 
			RPC_DBWEBTRADEPREPOST,
			DBWebTradePrePostArg(
				sn,
				proto.roleid,
				proto.userid,
				Octets(),//name
				4,//posttype
				0,//money
				0,//item_id,
				0,//item_pos,
				0,//item_num,
				proto.price,
				proto.sellperiod,
				proto.buyer_roleid,
				post_time,
				state,
				deposit,
				ui.ip,
				GMailSyncData()	
			)
		);
	rpc->save_linksid=ui.linksid;
	rpc->save_localsid=ui.localsid;
	rpc->save_gsid=_GAMESERVER_ID_INVALID;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

int WebTradeMarket::TryRolePreCancelPost(int roleid, const UserInfo& ui)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;
	//根据roleid找到sn
	RoleMap::iterator r_it = role_sell_map.find(roleid);
	if(r_it == role_sell_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	SNSet& sn_set = r_it->second;
	if(sn_set.size() != 1) return ERR_WT_ENTRY_NOT_FOUND;//寄售时限制了角色不能存在其他寄售
	int64_t sn = *(sn_set.begin());

	if(PostOffice::GetInstance().GetMailBoxSize(roleid) >= SYS_MAIL_LIMIT) return ERR_WT_MAILBOX_FULL;
	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	if(it->second->second.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(detail.info.state != WebTradeObj::STATE_POST) return ERR_WT_CANNOT_CANCELPOST;
	if(detail.info.posttype != 4) return ERR_WT_CANNOT_CANCELPOST;

	it->second->second.SetBusy(true);
	//query DB
	int state = WebTradeObj::STATE_PRE_CANCEL_POST;
	DBWebTradePreCancelPost * rpc = (DBWebTradePreCancelPost *)Rpc::Call(
			RPC_DBWEBTRADEPRECANCELPOST,
			DBWebTradePreCancelPostArg(
				sn,
				roleid,
				state
			)
		);	
	rpc->save_linksid=ui.linksid;
	rpc->save_localsid=ui.localsid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;	
}

void WebTradeMarket::GetWebTradeList(const find_param_t& param,std::vector<GWebTradeItem>& result,unsigned int& end)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return;
	
	//所有物品依次显示
	if(param.category==0)	
	{
		size_t szFoundNum = sn_map.size();
		if(szFoundNum == 0) return;
		SNMap::reverse_iterator it = sn_map.rbegin(), ite = sn_map.rend();
		size_t handle = param.handle;
		if(param.blForward && handle<szFoundNum)
		{
			std::advance(it,handle);
			size_t i,count;
			for(i=handle,count=0; i<szFoundNum && count<WEBTRADE_PAGE_SIZE; ++i,++it)
			{
				GWebTradeDetail & detail = it->second->second.GetDetail();
				if(detail.info.state == WebTradeObj::STATE_SHOW 
						|| detail.info.state == WebTradeObj::STATE_SELL && (detail.info.buyer_roleid==0 || detail.info.buyer_roleid==param.roleid))
				{
					result.push_back(detail.info);	
					++count;
				}	
			}
			end = i;
		}
		else if(!param.blForward)
		{
			if(handle >= szFoundNum) handle = szFoundNum-1;
			std::advance(it,handle);
			size_t i,count;
			for(i=handle,count=0; i!=(size_t)-1 && count<WEBTRADE_PAGE_SIZE; --i,--it)
			{
				GWebTradeDetail & detail = it->second->second.GetDetail();
				if(detail.info.state == WebTradeObj::STATE_SHOW 
						|| detail.info.state == WebTradeObj::STATE_SELL && (detail.info.buyer_roleid==0 || detail.info.buyer_roleid==param.roleid))
				{
					result.insert(result.begin(), detail.info);	
					++count;
				}	
			}
			end = i;
		}
		return;	
	}	

	//指定目录树内依次显示
	CategoryMap::iterator it = category_map.lower_bound(param.category),
							ite = category_map.upper_bound(param.category);
	size_t szFoundNum = ( it!=category_map.end() && (*it).first==param.category ) ? std::distance(it,ite) : 0;
	if(szFoundNum == 0)	return;
	size_t handle=param.handle;
	if(param.blForward && handle<szFoundNum)
	{
		std::advance(it,handle);
		size_t i,count;
		for(i=handle,count=0; i<szFoundNum && count<WEBTRADE_PAGE_SIZE; ++i,++it)
		{
			GWebTradeDetail & detail = it->second.GetDetail();
			if(detail.info.state == WebTradeObj::STATE_SHOW 
				|| detail.info.state == WebTradeObj::STATE_SELL && (detail.info.buyer_roleid==0 || detail.info.buyer_roleid==param.roleid))
			{
				result.push_back(detail.info);	
				++count;
			}	
		}
		end = i;
	}
	else if(!param.blForward)
	{
		if(handle >= szFoundNum) handle = szFoundNum-1;
		std::advance(it,handle);	
		size_t i,count;
		for(i=handle,count=0; i!=(size_t)-1 && count<WEBTRADE_PAGE_SIZE; --i,--it)
		{
			GWebTradeDetail & detail = it->second.GetDetail();
			if(detail.info.state == WebTradeObj::STATE_SHOW 
				|| detail.info.state == WebTradeObj::STATE_SELL && (detail.info.buyer_roleid==0 || detail.info.buyer_roleid==param.roleid))
			{
				result.insert(result.begin(), detail.info);	
				++count;
			}	
		}		
		end = i;		
	}
}

bool WebTradeMarket::GetWebTrade(int64_t sn, GWebTradeDetail& detail)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	detail = it->second->second.GetDetail();
	return true;
}

bool WebTradeMarket::GetRoleWebTrade(int roleid, GWebTradeDetail& detail)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;
	//根据roleid找到sn
	RoleMap::iterator r_it = role_sell_map.find(roleid);
	if(r_it == role_sell_map.end()) return false;
	SNSet& sn_set = r_it->second;
	if(sn_set.size() != 1) return false;//寄售时限制了角色不能存在其他寄售
	int64_t sn = *(sn_set.begin());

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	detail = it->second->second.GetDetail();
	return true;
}

void WebTradeMarket::GetAttendWebTradeList(int roleid, bool getsell, unsigned int begin, std::vector<GWebTradeItem>& result,unsigned int& end)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return;

	RoleMap::iterator r_it;
	if(getsell)
	{
		r_it = role_sell_map.find(roleid);
		if(r_it == role_sell_map.end()) return;
	}
	else
	{
		r_it = role_buy_map.find(roleid);
		if(r_it == role_buy_map.end()) return;
	}
	SNSet& sn_set = r_it->second;
	if(begin == (unsigned int)-1)
	{
		if(sn_set.size() >= WEBTRADE_PAGE_SIZE)
			begin = sn_set.size()-WEBTRADE_PAGE_SIZE;
		else
			begin = 0;
	}
	if(begin >= sn_set.size()) return;
	SNSet::iterator s_it = sn_set.begin(),s_ite = sn_set.end();
	std::advance(s_it, begin);
	end = begin;
	size_t count = 0;
	for( ; s_it != s_ite && count < WEBTRADE_PAGE_SIZE; ++s_it, ++end)
	{
		SNMap::iterator it = sn_map.find(*s_it);
		if(it != sn_map.end())
		{
			result.push_back(it->second->second.GetDetail().info);
			count ++;
		}
	}
	return;
}

void WebTradeMarket::SendPost(category_t c, WebTradeObj & obj)
{
	obj.SetBusy(true, 30000000);
	GWebTradeDetail & detail = obj.GetDetail();
	
	Post p;
	p.aid = aid;
	p.zoneid = zoneid;
	p.seller.roleid = detail.info.seller_roleid;
	p.seller.userid = detail.info.seller_userid;
	p.seller.rolename = detail.info.seller_name;
	p.buyer.roleid = detail.info.buyer_roleid;
	p.buyer.userid = detail.info.buyer_userid; 
	p.buyer.rolename = detail.info.buyer_name;
	p.sn = detail.info.sn;
	p.price = detail.info.price;
	p.shelf = detail.info.price>0 ? 1 : 0;
	p.posttype = detail.info.posttype;
	p.loginip = detail.loginip;
	p.time.actiontime = detail.post_time;
	p.time.showperiod = DEFAULT_SHOW_PERIOD;
	p.time.sellperiod =	detail.info.sell_endtime/60;
	p.time.postperiod = 0;		//非0时由游戏设置寄售时长 0时由平台设置寄售时长
	Marshal::OctetsStream os;
	if(detail.info.posttype == 1)
	{
		GRoleInventory m;
		m.id = MONEY_MATTER_ID;
		m.pos = c;
		m.count = detail.info.money;
		os << m;
		p.num = detail.info.money;
	}
	else if(detail.info.posttype == 2)
	{
		detail.item.pos = c;
		os << detail.item;
		p.num = detail.info.item_count;
	}
	else if(detail.info.posttype == 4)
	{
		os << (int)ROLE_DUMMY_ID << c;	//与物品金币保持一致吧
		os.insert(os.end(),detail.rolebrief.begin(),detail.rolebrief.size());
		p.num = 1;	
	}
	p.info.detail = os;
	if(detail.info.posttype != 4)	p.backup = os;	//对于角色来说，这个数据太大，不再设置了
	p.timestamp = 0; 
	GWebTradeClient::GetInstance()->SendProtocol(p);
}

void WebTradeMarket::RecvPostRe(bool success, int userid, int64_t sn, int postperiod, int showperiod, int commodity_id)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_userid != userid) return;
	if(detail.info.state != WebTradeObj::STATE_PRE_POST) return;

	if(!success)
	{
		it->second->second.SetBusy(true);	//设置busy的timeout为-1
		//query DB
		DBWebTradeCancelPost * rpc = (DBWebTradeCancelPost *)Rpc::Call(
			RPC_DBWEBTRADECANCELPOST,
			DBWebTradeCancelPostArg(
				sn,	
				detail.info.seller_roleid,
				1
			)
		);		
		rpc->web_op = false;
		rpc->userid = detail.info.seller_userid;
		rpc->posttype = detail.info.posttype;
		GameDBClient::GetInstance()->SendProtocol(rpc);
		return;	
	}

	it->second->second.SetBusy(true);	//设置busy的timeout为-1
	//query DB
	int state,post_endtime=0,show_endtime=0,sell_endtime=0;
	post_endtime = int(detail.post_time/1000) + postperiod*60;
	if(detail.info.price > 0)
	{
		if(showperiod > 0)
		{
			state = WebTradeObj::STATE_SHOW;
			show_endtime = int(detail.post_time/1000) + showperiod*60;
		}
		else
			state = WebTradeObj::STATE_SELL;		
		sell_endtime = int(detail.post_time/1000) + showperiod*60 + detail.info.sell_endtime;	//寄售前sell_endtime保存的是要上架的秒数
	}
	else
		state = WebTradeObj::STATE_POST;	
	DBWebTradePost * rpc=(DBWebTradePost*)Rpc::Call(
			RPC_DBWEBTRADEPOST,
			DBWebTradePostArg(
				sn,
				detail.info.seller_roleid,
				state,
				post_endtime,
				show_endtime,
				sell_endtime,
				commodity_id
				)
			);		
	GameDBClient::GetInstance()->SendProtocol(rpc);
}

bool WebTradeMarket::OnDBPost(int roleid, int64_t sn, int state, int post_endtime, int show_endtime, int sell_endtime, int commodity_id)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return false;

	detail.info.state = state;
	detail.info.post_endtime = post_endtime;
	detail.info.show_endtime = show_endtime;
	detail.info.sell_endtime = sell_endtime;
	detail.info.commodity_id = commodity_id;
	if(detail.info.posttype == 4 && detail.rolebrief.size() > 256)
	{
		//交易平台已寄售成功，游戏内精简一下数据
		try{
			GWebTradeRoleBrief brief;
			Marshal::OctetsStream(detail.rolebrief) >> brief;
			brief.petcorral.clear();
			brief.property.clear();
			brief.skills.clear();
			brief.inventory = GRolePocket();
			brief.equipment = GRoleEquipment();
			brief.storehouse = GRoleStorehouse();
			brief.force_data.clear();
			detail.rolebrief = Marshal::OctetsStream()<<brief;
		}catch(Marshal::Exception)
		{
			detail.rolebrief.clear();	
		}
	}
	it->second->second.SetBusy(false);
	return true;
}

void WebTradeMarket::SendCancelPost(WebTradeObj & obj)
{
	obj.SetBusy(true, 30000000);
	GWebTradeDetail & detail = obj.GetDetail();
	
	GamePostCancel p;
	p.userid	=	detail.info.seller_userid;
	p.roleid	=	detail.info.seller_roleid;
	p.sn		=	detail.info.sn;
	p.timestamp =	0;
	GWebTradeClient::GetInstance()->SendProtocol(p);
}

void WebTradeMarket::RecvCancelPostRe(bool success, int userid, int64_t sn)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_userid != userid) return;
	if(detail.info.state != WebTradeObj::STATE_PRE_CANCEL_POST) return;

	if(!success)
	{
		//重新post
		it->second->second.SetBusy(true);//设置busy的timeout为-1	
		//query DB
		int state = WebTradeObj::STATE_POST;
		DBWebTradePost * rpc=(DBWebTradePost*)Rpc::Call(
				RPC_DBWEBTRADEPOST,
				DBWebTradePostArg(
					sn,
					detail.info.seller_roleid,
					state,
					detail.info.post_endtime,
					detail.info.show_endtime,
					detail.info.sell_endtime,
					detail.info.commodity_id
					)
				);		
		GameDBClient::GetInstance()->SendProtocol(rpc);
		return;	
	}
	
	it->second->second.SetBusy(true);	//设置busy的timeout为-1
	//query DB
	DBWebTradeCancelPost * rpc = (DBWebTradeCancelPost *)Rpc::Call(
		RPC_DBWEBTRADECANCELPOST,
		DBWebTradeCancelPostArg(
			sn,	
			detail.info.seller_roleid,
			0
		)
	);		
	rpc->web_op = false;
	rpc->userid = detail.info.seller_userid;
	rpc->posttype = detail.info.posttype;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return;
}

bool WebTradeMarket::OnDBCancelPost(int roleid, int64_t sn)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	return RemoveItem(roleid,sn,true);
}

int WebTradeMarket::DoWebPostCancel(int userid, int roleid, int64_t sn, int ctype, int64_t messageid, int64_t timestamp)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	WebTradeObj & obj = it->second->second;
	if(obj.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = obj.GetDetail();
	//if(detail.info.seller_userid != userid) return ERR_WT_SN_USERID_MISMATCH;
	//if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(!obj.CheckTimestamp(timestamp)) return ERR_WT_TIMESTAMP_MISMATCH;
	
	obj.SetBusy(true);
	//query DB
	DBWebTradeCancelPost * rpc = (DBWebTradeCancelPost *)Rpc::Call(
		RPC_DBWEBTRADECANCELPOST,
		DBWebTradeCancelPostArg(
			sn,	
			detail.info.seller_roleid,
			(ctype==1 ? 1 : 0)  //ctype == 1 客服强行取消寄售,退保证金
		)
	);		
	rpc->web_op = true;
	rpc->messageid = messageid;
	rpc->timestamp = timestamp;
	rpc->userid = detail.info.seller_userid;
	rpc->userid_mismatch = (detail.info.seller_userid != userid);
	rpc->roleid_mismatch = (detail.info.seller_roleid != roleid);
	rpc->posttype = detail.info.posttype;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

int WebTradeMarket::DoShelf(int userid, int roleid, int64_t sn, int price, int64_t actiontime, int showperiod, int sellperiod, int buyer_roleid, int64_t messageid, int64_t timestamp)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	WebTradeObj & obj = it->second->second;
	if(obj.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = obj.GetDetail();
	//if(detail.info.seller_userid != userid) return ERR_WT_SN_USERID_MISMATCH;
	//if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(!obj.CheckTimestamp(timestamp)) return ERR_WT_TIMESTAMP_MISMATCH;
	
	obj.SetBusy(true);
	//query DB
	int state, show_endtime = 0, sell_endtime = 0;
	if(showperiod > 0)
	{
		state = WebTradeObj::STATE_SHOW;		
		show_endtime = int(actiontime/1000) + showperiod*60;
	}else
		state =  WebTradeObj::STATE_SELL;
	sell_endtime = int(actiontime/1000) + showperiod*60 + sellperiod*60;	
	DBWebTradeShelf * rpc = (DBWebTradeShelf *)Rpc::Call(
			RPC_DBWEBTRADESHELF,
			DBWebTradeShelfArg(
				sn,
				detail.info.seller_roleid,
				state,
				show_endtime,
				price,
				sell_endtime,
				buyer_roleid)
		);
	rpc->messageid = messageid;
	rpc->timestamp = timestamp;
	rpc->userid = detail.info.seller_userid;
	rpc->userid_mismatch = (detail.info.seller_userid != userid);
	rpc->roleid_mismatch = (detail.info.seller_roleid != roleid);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool WebTradeMarket::OnDBShelf(int roleid, int64_t sn, int state, int show_endtime, int price, int sell_endtime, int buyer_roleid, int buyer_userid, Octets& buyer_name, int64_t timestamp)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return false;

	if(detail.info.buyer_roleid > 0)
		role_buy_map[detail.info.buyer_roleid].erase(detail.info.sn);
	detail.info.state = state;
	detail.info.show_endtime = show_endtime;
	detail.info.price = price;
	detail.info.sell_endtime = sell_endtime;
	detail.info.buyer_roleid = buyer_roleid;
	detail.info.buyer_userid = buyer_userid;
	detail.info.buyer_name = buyer_name;
	if(detail.info.buyer_roleid > 0)
		role_buy_map[detail.info.buyer_roleid].insert(detail.info.sn);
	it->second->second.UpdateTimestamp(timestamp);
	it->second->second.SetBusy(false);	
	return true;
}

int WebTradeMarket::DoShelfCancel(int userid, int roleid, int64_t sn, int64_t messageid, int64_t timestamp)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	WebTradeObj & obj = it->second->second;
	if(obj.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = obj.GetDetail();
	//if(detail.info.seller_userid != userid) return ERR_WT_SN_USERID_MISMATCH;
	//if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(!obj.CheckTimestamp(timestamp)) return ERR_WT_TIMESTAMP_MISMATCH;
	
	obj.SetBusy(true);
	//query DB
	int state = WebTradeObj::STATE_POST;
	DBWebTradeCancelShelf * rpc = (DBWebTradeCancelShelf *)Rpc::Call(
			RPC_DBWEBTRADECANCELSHELF,
			DBWebTradeCancelShelfArg(
				sn,
				detail.info.seller_roleid,
				state)
		);
	rpc->messageid = messageid;
	rpc->timestamp = timestamp;
	rpc->userid = detail.info.seller_userid;
	rpc->userid_mismatch = (detail.info.seller_userid != userid);
	rpc->roleid_mismatch = (detail.info.seller_roleid != roleid);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool WebTradeMarket::OnDBCancelShelf(int roleid, int64_t sn, int64_t timestamp)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return false;
	GWebTradeDetail & detail = it->second->second.GetDetail();
	if(detail.info.seller_roleid != roleid) return false;

	if(detail.info.buyer_roleid > 0)
		role_buy_map[detail.info.buyer_roleid].erase(detail.info.sn);
	detail.info.state = WebTradeObj::STATE_POST;
	detail.info.show_endtime = 0;
	detail.info.price = 0;
	detail.info.sell_endtime = 0;
	detail.info.buyer_roleid = 0;
	detail.info.buyer_userid = 0;
	detail.info.buyer_name.clear();
	it->second->second.UpdateTimestamp(timestamp);
	it->second->second.SetBusy(false);	
	return true;
}

int WebTradeMarket::DoSold(int _zoneid, int userid, int roleid, int64_t sn, int buyer_userid, int buyer_roleid, int64_t orderid, int stype, int64_t timestamp)
{
	//no check zoneid
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;

	//检查物品是否已卖出
	if(sold_set.find(sn) != sold_set.end()) return ERR_WT_ENTRY_HAS_BEEN_SOLD;
	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	WebTradeObj & obj = it->second->second;
	if(obj.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = obj.GetDetail();
	//if(detail.info.seller_userid != userid) return ERR_WT_SN_USERID_MISMATCH;
	//if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(!obj.CheckTimestamp(timestamp)) return ERR_WT_TIMESTAMP_MISMATCH;
	if(buyer_roleid == detail.info.seller_roleid) return ERR_WT_BUYER_NOT_EXIST;
	if(detail.info.posttype == 4)
	{
		//买家不能在线，并需要锁定，防止在交易期间登陆. 由于买家以userid 为准，所以应检查锁定userid
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(buyer_userid);
		if(userinfo) return ERR_WT_BUYER_STATUS_INAPPROPRIATE; 
		//锁定该帐号一会
		ForbiddenUsers::GetInstance().Push(buyer_userid,buyer_roleid,_STATUS_OFFLINE);
	}
	
	obj.SetBusy(true);
	//query DB
	DBWebTradeSold* rpc = (DBWebTradeSold *)Rpc::Call(
			RPC_DBWEBTRADESOLD,
			DBWebTradeSoldArg(
				sn,
				detail.info.seller_roleid,
				buyer_roleid,
				buyer_userid)
		);
	rpc->zoneid = _zoneid;
	rpc->orderid = orderid;
	rpc->timestamp = timestamp;
	rpc->userid = detail.info.seller_userid;
	rpc->userid_mismatch = (detail.info.seller_userid != userid);
	rpc->roleid_mismatch = (detail.info.seller_roleid != roleid);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool WebTradeMarket::OnDBSold(int roleid, int64_t sn)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	if(!RemoveItem(roleid,sn,false)) return false;
	//将此sn加入到已卖出物品列表
	sold_set.insert(sn);
	return true;
}

int WebTradeMarket::DoPostExpire(int userid, int roleid, int64_t sn, int64_t messageid, int64_t timestamp)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return ERR_WT_UNOPEN;

	SNMap::iterator it = sn_map.find(sn);
	if(it == sn_map.end()) return ERR_WT_ENTRY_NOT_FOUND;
	WebTradeObj & obj = it->second->second;
	if(obj.IsBusy())	return ERR_WT_ENTRY_IS_BUSY;
	GWebTradeDetail & detail = obj.GetDetail();
	//if(detail.info.seller_userid != userid) return ERR_WT_SN_USERID_MISMATCH;
	//if(detail.info.seller_roleid != roleid) return ERR_WT_SN_ROLEID_MISMATCH;
	if(!obj.CheckTimestamp(timestamp)) return ERR_WT_TIMESTAMP_MISMATCH;
	
	obj.SetBusy(true);
	//query DB
	DBWebTradePostExpire* rpc = (DBWebTradePostExpire *)Rpc::Call(
			RPC_DBWEBTRADEPOSTEXPIRE,
			DBWebTradePostExpireArg(
				sn,
				detail.info.seller_roleid)
		);
	rpc->messageid = messageid;
	rpc->timestamp = timestamp;
	rpc->userid = detail.info.seller_userid;
	rpc->userid_mismatch = (detail.info.seller_userid != userid);
	rpc->roleid_mismatch = (detail.info.seller_roleid != roleid);
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool WebTradeMarket::OnDBPostExpire(int roleid, int64_t sn)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsMarketOpen())	return false;

	return RemoveItem(roleid,sn,true);
}

}
