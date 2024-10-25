#include "gmailsyncdata"
#include "createfactionfortress.hpp"
#include "notifyfactionfortressinfo2.hpp"
#include "factionfortresschallenge.hpp"
#include "notifyfactionfortressid.hpp"
#include "dbfactionfortressload.hrp"
#include "dbcreatefactionfortress.hrp"
#include "dbputfactionfortress.hrp"
#include "dbdelfactionfortress.hrp"
#include "dbfactionfortresschallenge.hrp"
#include "mapuser.h"
#include "factionfortressmanager.h"
#include "gfactionclient.hpp"
#include "maplinkserver.h"
#include "chatbroadcast.hpp"
#include "factionchat.hpp"

namespace GNET
{
	
void FactionFortressDetailToBrief(const GFactionFortressDetail & detail, GFactionFortressBriefInfo & brief)
{
	brief.factionid 		= detail.factionid;
	brief.level 			= detail.info.level;
	brief.building 			= detail.info.building;
	brief.health 			= detail.info2.health;
	brief.offense_faction 	= detail.info2.offense_faction;
}

void FactionFortressObj::SyncDB()
{
	if(detail.info2.health > 0)
	{
		DBPutFactionFortress * rpc = (DBPutFactionFortress *)Rpc::Call(RPC_DBPUTFACTIONFORTRESS,DBPutFactionFortressArg(detail));
		rpc->save_counter = change_counter;	
		GameDBClient::GetInstance()->SendProtocol(rpc);
		syncdb = true;
	}
	else if(!open)	//如基地已开启，等待基地关闭后再delete
	{
		DBDelFactionFortress * rpc = (DBDelFactionFortress *)Rpc::Call(RPC_DBDELFACTIONFORTRESS,DBDelFactionFortressArg(detail.factionid));
		GameDBClient::GetInstance()->SendProtocol(rpc);
		syncdb = true;
	}
}

void FactionFortressObj::OnDBSync(size_t save_counter)
{
	if(syncdb)
	{
		syncdb = false;
		if(save_counter == change_counter)
		{
			change_counter = 0;
		}
	}
	else
	{
		Log::log(LOG_ERR,"factionfortress OnDBSync SYNC STATE ERROR. factionid=%d", detail.factionid);
	}
}

void FactionFortressObj::SyncGS(int server_id)
{
	if(GProviderServer::GetInstance()->DispatchProtocol(server_id,NotifyFactionFortressInfo2(detail.factionid,detail.info2)))
	{
		LOG_TRACE("factionfortress SyncGS. factionid=%d", detail.factionid);
		needsyncgs = false;
	}
}
	
bool FactionFortressMan::Initialize()
{ 
	//每天23:50刷新健康度
	time_t t = time(NULL);
	struct tm & tm1 = *localtime(&t);
	health_update_time = t - tm1.tm_hour*3600 - tm1.tm_min*60 - tm1.tm_sec - 600;
	if(t - health_update_time >= 86400)
		health_update_time += 86400;
	
	//本周周起始时间
	t_base = t - 86400*tm1.tm_wday - 3600*tm1.tm_hour - 60*tm1.tm_min - tm1.tm_sec;
	
	/*暂时不开放战斗,去掉战斗时间即可
	//战斗时间配置
	//周一19:00-23:00 宣战 周二 21:30-23:00 战斗
	bp_list.push_back(BATTLE_PERIOD(86400+19*3600, 86400+23*3600, 86400*2+21*3600+30*60, 86400*2+23*3600));
	//周三19:00-23:00 宣战 周四 21:30-23:00 战斗
	bp_list.push_back(BATTLE_PERIOD(86400*3+19*3600, 86400*3+23*3600, 86400*4+21*3600+30*60, 86400*4+23*3600));
	*/	
	IntervalTimer::Attach( this,FACTION_FORTRESS_UPDATE_INTERVAL/IntervalTimer::Resolution() );	
	return true; 
}

void FactionFortressMan::OnDBConnect(Protocol::Manager * manager, int sid)
{
	Thread::Mutex::Scoped l(lock);
	if(status == ST_INIT)
		manager->Send(sid, Rpc::Call(RPC_DBFACTIONFORTRESSLOAD,DBFactionFortressLoadArg()));
}

void FactionFortressMan::OnDBLoad(const std::vector<GFactionFortressDetail>& list, bool finish)
{
	Thread::Mutex::Scoped l(lock);
	if(status == ST_INIT)
	{
		for(size_t i=0; i<list.size(); i++)
		{
			const GFactionFortressDetail & detail = list[i];
			fortress_map[detail.factionid] = FactionFortressObj(detail);	
		}

		if(finish)
		{
			Log::formatlog("factionfortress","initfactionfortress: total=%d", fortress_map.size()); 
			//数据接收完毕，进行一些初始化处理
			FinalInit();
			SyncFactionServer();
		}
	}
}

void FactionFortressMan::FinalInit()
{
	time_t t = time(NULL);
	UpdateTime(t);
	UpdateStatus(t);
	//试图根据战斗数据找到停机前的status
	int pre_status = ST_OPEN;
	FortressMap::iterator it = fortress_map.begin();
	for(; it!=fortress_map.end(); ++it)
	{
		GFactionFortressDetail & detail = it->second.GetDetail();
		if(detail.info2.challenge_list.size())
		{
			pre_status = ST_CHALLENGE;
			break;
		}
		if(detail.info2.offense_faction != 0)
		{
			pre_status = ST_BATTLE_WAIT;	//实际上还可能是ST_BATTLE
			break;
		}
	}
	if(pre_status == ST_CHALLENGE)
	{
		if(status == ST_OPEN)
		{
			//应该清除所有战斗数据
			BattleClear("FinalInit1");
		}
		else if(status == ST_BATTLE_WAIT)
		{
			ChallengeEnd();
		}
		else if(status == ST_BATTLE)
		{
			ChallengeEnd();
			BattleStart();
		}		
	}
	else if(pre_status == ST_BATTLE_WAIT)
	{
		if(status == ST_OPEN)	
		{
			//应该清除所有战斗数据
			BattleClear("FinalInit2");
		}
		else if(status == ST_CHALLENGE)
		{
			//应该清除所有战斗数据
			BattleClear("FinalInit3");
		}
		else if(status == ST_BATTLE)
		{
			BattleStart();
		}
	}
}

void FactionFortressMan::UpdateTime(int cur_t)
{
	while(cur_t - t_base >= 604800)
	{
		t_base += 604800;
	}
}

void FactionFortressMan::UpdateStatus(int cur_t)
{
	cur_t = cur_t - t_base;
	size_t i=0;
	for(; i<bp_list.size(); i++)
	{
		if(cur_t < bp_list[i].challenge_start_time)
		{
			status = ST_OPEN;
			break;
		}
		if(cur_t >= bp_list[i].battle_end_time)
			continue;
		if(cur_t < bp_list[i].challenge_end_time)
		{
			status = ST_CHALLENGE;
			break;
		}
		if(cur_t < bp_list[i].battle_start_time)
		{
			status = ST_BATTLE_WAIT;
			break;
		}
		if(cur_t < bp_list[i].battle_end_time)
		{
			status = ST_BATTLE;
			break;
		}
	}
	if(i == bp_list.size()) status = ST_OPEN;
}

void FactionFortressMan::GetNextBattleTime(int cur_t, int& start, int& end)
{
	cur_t = cur_t - t_base;
	size_t i=0;
	for(; i<bp_list.size(); i++)
	{
		if(cur_t < bp_list[i].battle_start_time)
		{
			start = t_base + bp_list[i].battle_start_time;
			end = t_base + bp_list[i].battle_end_time;
			break;
		}
	}
	if(i == bp_list.size())
	{
		start = end = 0;	
	}
}

void FactionFortressMan::BattleClear(const char * msg)
{
	FortressMap::iterator it = fortress_map.begin();
	for(; it!=fortress_map.end(); ++it)
	{
		FactionFortressObj & obj = it->second;
		GFactionFortressDetail & detail = obj.GetDetail();
		if(detail.info2.offense_faction != 0)
		{
			Log::log(LOG_ERR,"FortressBattleClear(%s): offense_faction!=0. factionid=%d,offense_faction=%d,starttime=%d,endtime=%d",
					msg, it->first, detail.info2.offense_faction,detail.info2.offense_starttime,detail.info2.offense_endtime);
			detail.info2.offense_faction = 0;
			detail.info2.offense_starttime = 0;
			detail.info2.offense_endtime = 0;
			obj.IncChangeFlag();
			obj.SetNeedSyncGS(true);
		}
		if(detail.info2.challenge_list.size())
		{
			Log::log(LOG_ERR,"FortressBattleClear(%s): challenge_list!=0. factionid=%d,challenge_list.size=%d",
					msg, it->first, detail.info2.challenge_list.size());
			detail.info2.challenge_list.clear();	
			obj.IncChangeFlag();
			obj.SetNeedSyncGS(true);
		}
	}
}

void FactionFortressMan::ChallengeStart()
{
	LOG_TRACE("factionfortress ChallengeStart.");
	//广播通告一下
	ChatBroadCast cbc;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = CMSG_FF_CHALLENGESTART; 
	LinkServer::GetInstance().BroadcastProtocol(cbc);
	//以防万一清除一下宣战列表
	BattleClear("ChallengeStart");
}

void FactionFortressMan::ChallengeEnd()
{
	LOG_TRACE("factionfortress ChallengeEnd.");
	//生成战斗列表
	//计算此次宣战对应的战斗时间
	time_t t = time(NULL);
	int battlestart=0, battleend=0;
	GetNextBattleTime(t,battlestart,battleend);
	FortressMap::iterator it = fortress_map.begin();
	for(; it!=fortress_map.end(); ++it)
	{
		FactionFortressObj & obj = it->second;
		GFactionFortressDetail & detail = obj.GetDetail();
		std::vector<int> & list = detail.info2.challenge_list;
		if(list.size())
		{
			detail.info2.offense_faction = list[rand()%list.size()];	
			detail.info2.offense_starttime = battlestart;
			detail.info2.offense_endtime = battleend;
			list.clear();
			obj.IncChangeFlag();
			obj.SetNeedSyncGS(true);

			FactionChat chat;
			chat.channel = GN_CHAT_CHANNEL_SYSTEM;
			chat.src_roleid = CMSG_FF_BECHALLENGED;
			chat.msg = Marshal::OctetsStream() << detail.info2.offense_faction;
			chat.dst_localsid = detail.factionid;
			GFactionClient::GetInstance()->SendProtocol(chat);

			FactionChat chat2;
			chat2.channel = GN_CHAT_CHANNEL_SYSTEM;
			chat2.src_roleid = CMSG_FF_CHALLENGESUCCESS;
			chat2.msg = Marshal::OctetsStream() << detail.factionid;
			chat2.dst_localsid = detail.info2.offense_faction;
			GFactionClient::GetInstance()->SendProtocol(chat2);
		}
	}	
}

void FactionFortressMan::BattleStart()
{
	LOG_TRACE("factionfortress BattleStart.");
	//广播通告一次
	ChatBroadCast cbc;
	cbc.channel = GN_CHAT_CHANNEL_SYSTEM;
	cbc.srcroleid = CMSG_FF_BATTLESTART; 
	LinkServer::GetInstance().BroadcastProtocol(cbc);

	FortressMap::iterator it = fortress_map.begin();
	for(; it!=fortress_map.end(); ++it)
	{
		FactionFortressObj & obj = it->second;
		GFactionFortressDetail & detail = obj.GetDetail();
		if(detail.info2.offense_faction)
		{
			FactionChat chat;
			chat.channel = GN_CHAT_CHANNEL_SYSTEM;
			chat.src_roleid = CMSG_FF_BATTLESTARTNOTIFY;
			chat.msg = Marshal::OctetsStream() << detail.info2.offense_faction;
			chat.dst_localsid = detail.factionid;
			GFactionClient::GetInstance()->SendProtocol(chat);
		}
	}
}

void FactionFortressMan::BattleEnd()
{
	LOG_TRACE("factionfortress BattleEnd.");
	FortressMap::iterator it = fortress_map.begin();
	for(; it!=fortress_map.end(); ++it)
	{
		FactionFortressObj & obj = it->second;
		GFactionFortressDetail & detail = obj.GetDetail();
		if(detail.info2.offense_faction)
		{
			FactionChat chat;
			chat.channel = GN_CHAT_CHANNEL_SYSTEM;
			chat.src_roleid = CMSG_FF_BATTLEENDNOTIFY;
			chat.msg = Marshal::OctetsStream() << detail.info2.offense_faction;
			chat.dst_localsid = detail.factionid;
			GFactionClient::GetInstance()->SendProtocol(chat);
			
			detail.info2.offense_faction = 0;
			detail.info2.offense_starttime = 0;
			detail.info2.offense_endtime = 0;
			obj.IncChangeFlag();
			obj.SetNeedSyncGS(true);
		}
	}
}
	
bool FactionFortressMan::Update()
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return true;

	time_t t = time(NULL);
	//更新周起始时间	
	UpdateTime(t);
	//更新当前status
	int old_status = status;
	UpdateStatus(t);
	if(status != old_status)
	{
		if(old_status == ST_OPEN && status == ST_CHALLENGE)
			ChallengeStart();
		else if(old_status == ST_CHALLENGE && status == ST_BATTLE_WAIT)
			ChallengeEnd();
		else if(old_status == ST_BATTLE_WAIT && status == ST_BATTLE)
			BattleStart();
		else if(old_status == ST_BATTLE && status == ST_OPEN)
			BattleEnd();
	}
	
	//更新健康度
	if(t - health_update_time >= 86400)
	{
		FortressMap::iterator it = fortress_map.begin();
		for(; it != fortress_map.end(); ++it)
		{
			FactionFortressObj & obj = it->second;
			GFactionFortressDetail & detail = obj.GetDetail();
			//int new_health = detail.info2.health - 1;
			int new_health = 150;
			if(detail.info.exp_today && detail.info.exp_today_time > health_update_time)
			{
				static const int table[] = {5000,15000,30000,50000,80000};
				int i = 0;
				while(i<(int)(sizeof(table)/sizeof(int)) && detail.info.exp_today >= table[i]) ++i;
				new_health += i;
			}
			if(new_health < 0) new_health = 0;
			if(new_health > 150) new_health = 150;
			
			if(detail.info2.health > 30 && new_health <= 30 
					|| new_health == 0)
			{
				FactionChat chat;
				chat.channel = GN_CHAT_CHANNEL_SYSTEM;
				chat.src_roleid = CMSG_FF_HEALTHNOTIFY;
				chat.msg = Marshal::OctetsStream() << new_health;
				chat.dst_localsid = detail.factionid;
				GFactionClient::GetInstance()->SendProtocol(chat);
			}
			
			detail.info2.health = new_health;
			obj.IncChangeFlag();
			obj.SetNeedSyncGS(true);
		}	
		health_update_time += 86400;
	}
	//存盘&通知gs
	{
		int update_count = 0;
		FortressMap::iterator it = fortress_map.lower_bound(update_cursor);
		for(; it != fortress_map.end() && update_count < FACTION_FORTRESS_CHECKSUM_ONUPDATE; ++it, ++update_count)
		{
			FactionFortressObj & obj = it->second;
			if(obj.NeedSyncDB())
			{
				obj.SyncDB();
			}
			if(obj.NeedSyncGS())
			{
				obj.SyncGS(server_id);
			}
		}
		if(it != fortress_map.end())
			update_cursor = it->first;
		else
			update_cursor = 0;
	}	
	
	return true;
}

void FactionFortressMan::RegisterServer(int _server_id, int _world_tag)
{
	Thread::Mutex::Scoped l(lock);
	server_id = _server_id;
	world_tag = _world_tag;
}

int FactionFortressMan::GameGetFortress(int factionid, GFactionFortressDetail & detail)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return ERR_FF_UNOPEN;

	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return ERR_FF_FORTRESS_NOT_EXIST;
	FactionFortressObj & obj = it->second;
	if(!obj.IsActive()) return ERR_FF_FORTRESS_DESTROYED; 
	detail = obj.GetDetail();
	return ERR_SUCCESS;
}

int FactionFortressMan::GamePutFortress(int factionid, const GFactionFortressInfo & info)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return ERR_FF_UNOPEN;

	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return ERR_FF_FORTRESS_NOT_EXIST;
	FactionFortressObj & obj = it->second;
	GFactionFortressDetail & detail = obj.GetDetail();
	detail.info = info;
	obj.IncChangeFlag();
	obj.SetOpen(true);	//重复设置一下
	return ERR_SUCCESS;
}

void FactionFortressMan::GameNotifyFortressState(int factionid, int state)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return;

	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return;
	it->second.SetOpen(state==1);
}

void FactionFortressMan::SyncFactionServer()
{
	if(!IsOpen()) return;
	LOG_TRACE( "FactionFortressMan: Sync faction id to gfactiond.");
	NotifyFactionFortressID proto;
	FortressMap::iterator it = fortress_map.begin();
	for( ; it!=fortress_map.end(); ++it)
	{
		GFactionFortressDetail & detail = it->second.GetDetail();
		proto.factionids.insert(detail.factionid);
		if(detail.info2.offense_faction > 0) proto.factionids.insert(detail.info2.offense_faction);
	}
	if(proto.factionids.size())
		GFactionClient::GetInstance()->SendProtocol(proto);
}

int FactionFortressMan::TryCreateFortress(const CreateFactionFortress & proto, const GFactionFortressInfo & info, const PlayerInfo & ui, const GMailSyncData & sync)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return ERR_FF_UNOPEN;
	
	if(fortress_map.find(proto.factionid) != fortress_map.end()) return ERR_FF_FORTRESS_ALREADY_EXIST;
	GFactionFortressDetail detail;
	detail.factionid = proto.factionid;
	detail.info = info;
	detail.info2.health = 100;	//初始健康度
	DBCreateFactionFortress * rpc = (DBCreateFactionFortress *)Rpc::Call(
			RPC_DBCREATEFACTIONFORTRESS,
			DBCreateFactionFortressArg(
				proto.roleid,
				proto.item_cost,
				detail,
				sync
			)
		);
	rpc->save_linksid=ui.linksid;
	rpc->save_localsid=ui.localsid;
	rpc->save_gsid=ui.gameid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool FactionFortressMan::OnDBCreateFortress(const GFactionFortressDetail & detail)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;
	
	if(fortress_map.find(detail.factionid) != fortress_map.end()) return false;
	fortress_map[detail.factionid] = FactionFortressObj(detail);

	FactionChat chat;
	chat.channel = GN_CHAT_CHANNEL_SYSTEM;
	chat.src_roleid = CMSG_FF_CREATE;
	chat.dst_localsid = detail.factionid;
	GFactionClient::GetInstance()->SendProtocol(chat);
	return true;
}

bool FactionFortressMan::OnDBPutFortress(int factionid, size_t save_counter)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;
	
	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return false;
	it->second.OnDBSync(save_counter);
	return true;
}

bool FactionFortressMan::OnDBDelFortress(int factionid)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;
	
	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return false;
	fortress_map.erase(it);
	return true;
}

void FactionFortressMan::OnDBSyncFailed(int factionid)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return;
	
	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return;
	it->second.OnDBSync(0);
}

bool FactionFortressMan::CheckEnterFortress(int factionid, int dst_factionid, int & dst_tag)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;
	
	//自己得有基地	
	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return false;
	if(!it->second.IsActive()) return false;
	
	if(factionid == dst_factionid)
	{
		//进入自己的帮派基地
		dst_tag = world_tag;
		return true;
	}
	//目的基地存在,且在战斗中且是进攻帮派
	FortressMap::iterator it2 = fortress_map.find(dst_factionid);
	if(it2 == fortress_map.end()) return false;
	if(!it2->second.IsActive()) return false;
	FactionFortressObj & obj2 = it2->second;
	GFactionFortressDetail & detail2 = obj2.GetDetail();
	if(status != ST_BATTLE || factionid != detail2.info2.offense_faction)
		return false;
	dst_tag = world_tag;
	return true;
}

void FactionFortressMan::GetFortressList(unsigned int & begin, int & st, std::vector<GFactionFortressBriefInfo> & list)
{
	Thread::Mutex::Scoped l(lock);
	st = status;
	if(!IsOpen()) return;

	if(begin == (unsigned int)-1)
	{
		if(fortress_map.size() >= FACTION_FORTRESS_PAGE_SIZE)
			begin = fortress_map.size() - FACTION_FORTRESS_PAGE_SIZE;
		else
			begin = 0;
	}
	if(begin >= fortress_map.size()) return;
	FortressMap::iterator it = fortress_map.begin();
	std::advance(it,begin);
	for( ; it!=fortress_map.end() && list.size()<FACTION_FORTRESS_PAGE_SIZE; ++it)
	{
		GFactionFortressDetail & detail = it->second.GetDetail();
		GFactionFortressBriefInfo brief;
		FactionFortressDetailToBrief(detail,brief);
		list.push_back(brief);
	}
}

int FactionFortressMan::TryChallenge(const FactionFortressChallenge & proto, const PlayerInfo & ui, const GMailSyncData & sync)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return ERR_FF_UNOPEN;

	if(status != ST_CHALLENGE) return ERR_FF_CANNOT_CHALLENGE; 
	if(proto.factionid == proto.target_factionid) return ERR_FF_CANNOT_CHALLENGE;
	//自己必须有基地
	FortressMap::iterator it = fortress_map.find(proto.factionid);
	if(it ==  fortress_map.end()) return ERR_FF_FORTRESS_NOT_EXIST;
	if(!it->second.IsActive()) return ERR_FF_FORTRESS_DESTROYED;
	//目标基地必须存在,且不能处于同步中
	FortressMap::iterator it2 = fortress_map.find(proto.target_factionid);
	if(it2 == fortress_map.end()) return ERR_FF_FORTRESS_NOT_EXIST;
	FactionFortressObj & obj2 = it2->second;	
	if(!obj2.IsActive()) return ERR_FF_FORTRESS_DESTROYED;
	if(obj2.InSyncDB()) return ERR_FF_FORTRESS_IN_SYNC; 
	GFactionFortressDetail & detail2 = obj2.GetDetail();
	std::vector<int> & list = detail2.info2.challenge_list;
	if(list.size() >= 32) return ERR_FF_CANNOT_CHALLENGE;
	if(std::find(list.begin(),list.end(),proto.factionid) != list.end()) return ERR_FF_TARGET_ALREADY_CHALLENGED;
	//
	obj2.SetSyncDB(true);
	DBFactionFortressChallenge * rpc = (DBFactionFortressChallenge *)Rpc::Call(
				RPC_DBFACTIONFORTRESSCHALLENGE,
				DBFactionFortressChallengeArg(
					proto.roleid,
					proto.factionid,
					proto.target_factionid,
					sync
				)			
			);
	rpc->save_linksid=ui.linksid;
	rpc->save_localsid=ui.localsid;
	rpc->save_gsid=ui.gameid;
	GameDBClient::GetInstance()->SendProtocol(rpc);
	return ERR_SUCCESS;
}

bool FactionFortressMan::OnDBChallenge(int factionid, int target_factionid)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;

	FortressMap::iterator it = fortress_map.find(target_factionid);
	if(it == fortress_map.end()) return false;
	FactionFortressObj & obj = it->second;
	GFactionFortressDetail & detail = obj.GetDetail();
	detail.info2.challenge_list.push_back(factionid);
	obj.OnDBSync(0);
	//要不要对自己帮喊一下
	return true;
}

void FactionFortressMan::OnDelFaction(int factionid)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return;

	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return;
	//将健康度置为0,基地将自动删除
	FactionFortressObj & obj = it->second;
	GFactionFortressDetail & detail = obj.GetDetail();
	detail.info2.health = 0;
	obj.IncChangeFlag();
	obj.SetNeedSyncGS(true);
}

void FactionFortressMan::GetBattleList(int& st, std::vector<GFactionFortressBattleInfo>& list)
{
	Thread::Mutex::Scoped l(lock);
	st = status;
	if(!IsOpen()) return;
	
	FortressMap::iterator it = fortress_map.begin();
	for( ; it!=fortress_map.end(); ++it)
	{
		GFactionFortressDetail & detail = it->second.GetDetail();
		if(detail.info2.offense_faction > 0)
			list.push_back(GFactionFortressBattleInfo(it->first,detail.info2.offense_faction)); 
	}
}

bool FactionFortressMan::GetFortress(int factionid, GFactionFortressBriefInfo & brief)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;

	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return false;
	GFactionFortressDetail & detail = it->second.GetDetail();
	FactionFortressDetailToBrief(detail,brief);
	return true;
}

bool FactionFortressMan::DebugDecHealthUpdateTime()
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;

	health_update_time -= 86400;
	return true;
}

void FactionFortressMan::DebugAdjustBattlePeriod(bool fastmode)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return;

	bp_list.clear();
	if(fastmode)
	{
		for(size_t i=0; i<2*7*24; i++)
			bp_list.push_back(BATTLE_PERIOD(i*1800,i*1800+300,i*1800+900,i*1800+1500));
	}
	else
	{
		bp_list.push_back(BATTLE_PERIOD(86400+19*3600, 86400+23*3600, 86400*2+21*3600+30*60, 86400*2+23*3600));
		bp_list.push_back(BATTLE_PERIOD(86400*3+19*3600, 86400*3+23*3600, 86400*4+21*3600+30*60, 86400*4+23*3600));
	}
	BattleClear("debug");
	time_t t = time(NULL);
	UpdateStatus(t);
}

bool FactionFortressMan::GetFactionExt(int factionid,FactionExt & fe)
{
	Thread::Mutex::Scoped l(lock);
	if(!IsOpen()) return false;
	FortressMap::iterator it = fortress_map.find(factionid);
	if(it == fortress_map.end()) return false;
	GFactionFortressDetail & detail = it->second.GetDetail();
	//fe.level =  
	fe.exp = detail.info.exp;
	fe.fortress_lvl = detail.info.level;
	fe.health = detail.info2.health;
	return true;
}
}
