#include "crosssystem.h"
#include "gdeliveryserver.hpp"
#include "senddataandidentity.hpp"
#include "saveplayerdata.hrp"
#include "senddataandidentity_re.hpp"
#include "remoteloginquery.hpp"
#include "remoteloginquery_re.hpp"
#include "freezeplayerdata.hrp"
#include "getremoteroleinfo.hpp"
#include "getroleinfo.hrp"
#include "addictioncontrol.hpp"
#include "uniquedataserver.h"
#include "crossguardnotify.hpp"
#include "cdcmnfbattleman.h"
#include "crosschat.hpp"
#include "crosschat_re.hpp"
#include "worldchat.hpp"
#include "crosssolochallengerank.hpp"
#include "crosssolochallengerank_re.hpp"
#include "solochallengerank.h"

namespace GNET
{
std::map<int, RoleInfo> DelayRolelistTask::roleinfo_map;

UserIdentityCache::UserIdentityCache()
{
	int timeout = atoi(Conf::GetInstance()->find(GDeliveryServer::GetInstance()->Identification(), "user_iden_cache_time").c_str());
	cache_max_time = timeout > DEFAULT_CACHE_MAXTIME ? timeout : DEFAULT_CACHE_MAXTIME;
	LOG_TRACE("UserIdentityCache cache time %d", cache_max_time);
	Timer::Attach(this); 
}


void DelayRolelistTask::OnRecvInfo(int uid, int rid)
{
	UserInfo* user = UserContainer::GetInstance().FindUser(uid);
	
	if(user != NULL) {
		std::map<int, RoleInfo>::iterator it = roleinfo_map.find(rid);
		if(it == roleinfo_map.end()) return;

		LOG_TRACE("DelayRolelistTask, userid=%d, roleid=%d",uid, rid);
		RoleInfo& info = it->second;
		
		RoleInfoVector rolelist;
		rolelist.add(info);

		RoleList_Re re(ERR_SUCCESS, (rid%MAX_ROLE_COUNT), uid, user->localsid, rolelist);
		GDeliveryServer::GetInstance()->Send(user->linksid, re);

		roleinfo_map.erase(rid);
	}
}

//协议处理函数 begin
void SendDataAndIdentity::Process(Manager *manager, Manager::Session::ID sid)
{
	//由于原服和跨服的roleid和remote_roleid正好相反，所以收到该协议时，要交换二者的位置
	int tmp = remote_roleid;
	remote_roleid = roleid;
	roleid = tmp;

	LOG_TRACE("CrossRelated Recv SendDataAndIdentity from zoneid %d roleid %d remote_roleid %d userid %d ip %d flag %d version %d logintime %d au_isgm %d au_func %d au_funcparm %d auth.size %d",
			src_zoneid, roleid, remote_roleid, userid, ip, flag, data_timestamp, logintime, au_IsGM, au_func, au_funcparm, auth.size());

	if(flag == DS_TO_CENTRALDS || flag == DIRECT_TO_CENTRALDS) {
		if(!GDeliveryServer::GetInstance()->IsCentralDS()) return;
		if(!CentralDeliveryServer::GetInstance()->IsConnect(src_zoneid)) return;
	} else if(flag == CENTRALDS_TO_DS) {
		if(GDeliveryServer::GetInstance()->IsCentralDS()) return;
	} else {
		return;
	}

	SendDataAndIdentity_Re re(-1, roleid, remote_roleid, userid, flag, GDeliveryServer::GetInstance()->GetZoneid());

	UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
	
	//原服->跨服时，pinfo为NULL
	//跨服->原服时，pinfo不为NULL, 同时pinfo->status应该是_STATUS_REMOTE_LOGIN
	if(pinfo /*&& !arg.blkickuser*/ && pinfo->status != _STATUS_REMOTE_LOGIN) {
		Log::log(LOG_ERR, "CrossRelated SendDataAndIdentity roleid %d, remote_roleid %d userid %d already online status %d", roleid, remote_roleid, userid, pinfo->status);
		re.retcode = ERR_MULTILOGIN;
		manager->Send(sid, re);
		return;
	}
	
	//这里处理的是跨服->原服时，将原服里的user先logout
	if(pinfo) {
		UserContainer::GetInstance().UserLogout(pinfo, 0, true); //pinfo 析构
	}

	if(ForbiddenUsers::GetInstance().IsExist(userid)) {
		Log::log(LOG_ERR, "CrossRelated SendDataAndIdentity roleid %d remote_roleid %d is handling by GS", roleid, remote_roleid);
		re.retcode = ERR_ACCOUNTLOCKED;
		manager->Send(sid, re);
		return;
	}

	if(RemoteLoggingUsers::GetInstance().IsExist(userid)) {
		Log::log(LOG_ERR, "CrossRelated SendDataAndIdentity user %d is in remote logging process", userid);
		re.retcode = ERR_ACCOUNTLOCKED;
		manager->Send(sid, re);
		return;
	}

	if(UserIdentityCache::GetInstance()->Exist(userid)) {
		Log::log(LOG_ERR, "CrossRelated UserIdentityCache userid %d already exists", userid);
		manager->Send(sid, re);
		return;
	}

	if(!GameDBClient::GetInstance()->SendProtocol((SavePlayerData*)Rpc::Call(RPC_SAVEPLAYERDATA, SavePlayerDataArg(roleid, remote_roleid, userid, src_zoneid, data, flag, data_timestamp)))) {
		Log::log(LOG_ERR, "CrossRelted SendDataAndIdentity Send to SavePlayerData error roleid %d remote_roleid %d userid %d zoneid %d", roleid, remote_roleid, userid, src_zoneid);
		re.retcode = ERR_GAMEDB_FAIL;
		manager->Send(sid, re);
		return;
	}

	UserIdentityCache::GetInstance()->Insert(userid, 
		UserIdentityCache::Identity(roleid, remote_roleid, src_zoneid, ip, iseckey, oseckey, account, random, logintime, au_IsGM, au_func, au_funcparm, auth, usbbind, reward_mask, forbid_talk));

	LOG_TRACE("CrossRelated Send to SavePlayerData roleid %d remote_roleid %d userid %d", roleid, remote_roleid, userid);
}

void RemoteLoginQuery::Process(Manager *manager, Manager::Session::ID sid)
{
	//由于原服和跨服的roleid和remote_roleid正好相反，所以收到该协议时，要交换二者的位置
	int tmp = remote_roleid;
	remote_roleid = roleid;
	roleid = tmp;

	LOG_TRACE("Recv RemoteLoginQuery retcode %d roleid %d remote_roleid %d userid %d flag %d", retcode, roleid, remote_roleid, userid, flag);
	if(GDeliveryServer::GetInstance()->IsCentralDS()) return;
	
	RemoteLoginQuery_Re re(ERR_SUCCESS, roleid, remote_roleid, userid, flag);
	
	UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
	if(pinfo == NULL || pinfo->status != _STATUS_REMOTE_HALFLOGIN) {
		Log::log(LOG_ERR, "RemoteLoginQuery timeout userid %d userstatus %d", userid, pinfo == NULL ? 0: pinfo->status);
		re.retcode = 101;
		manager->Send(sid, re);
		return;
	}
	
	if(retcode == ERR_SUCCESS) {
		if(flag == DS_TO_CENTRALDS) {
			FreezePlayerData* rpc = (FreezePlayerData*)Rpc::Call(RPC_FREEZEPLAYERDATA, FreezePlayerDataArg(roleid, remote_roleid, userid, remote_zoneid));
			
			if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
				Log::log(LOG_ERR, "RemoteLoginQuery FreezePlayerData Failed userid %d roleid %d", userid, roleid);
				re.retcode = 102;
				manager->Send(sid, re);

				RemoteLoggingUsers::GetInstance().Pop(userid);
				if(pinfo->actime > 0 && pinfo->acstate) {
					CentralDeliveryClient::GetInstance()->SendProtocol(pinfo->acstate);
				}
				UserContainer::GetInstance().UserLogout(pinfo);
				return;
			} else {
				LOG_TRACE("RemoteLoginQuery try to FreezePlayerData roleid %d", roleid);
			}
		} else {
			if(manager->Send(sid, re)) {
				LOG_TRACE("Send RemoteLoginQuery_Re retcode %d roleid %d userid %d",
						re.retcode, roleid, userid);
				pinfo->status = _STATUS_REMOTE_LOGIN;
				UserContainer::GetInstance().InsertRemoteOnline(userid);
			} else {
				if(pinfo->actime > 0 && pinfo->acstate) {
					CentralDeliveryClient::GetInstance()->SendProtocol(pinfo->acstate);
				}
				UserContainer::GetInstance().UserLogout(pinfo);
				return;
			}
			
			RemoteLoggingUsers::GetInstance().Pop(userid);
		}
		
		if(pinfo->actime > 0 && pinfo->acstate) {
			CentralDeliveryClient::GetInstance()->SendProtocol(pinfo->acstate);
		}
		
	} else {
		RemoteLoggingUsers::GetInstance().Pop(userid);
		UserContainer::GetInstance().UserLogout(pinfo);
	}
}

void GetRemoteRoleInfo::Process(Manager *manager, Manager::Session::ID sid)
{
	//由于原服和跨服的roleid和remote_roleid正好相反，所以收到该协议时，要交换二者的位置
	int tmp = remote_roleid;
	remote_roleid = roleid;
	roleid = tmp;

	LOG_TRACE("Recv GetRemoteRoleInfo roleid %d remote_roleid %d userid %d zoneid %d", roleid, remote_roleid, userid, zoneid);
	
	GetRoleInfo* rpc = (GetRoleInfo*) Rpc::Call(RPC_GETROLEINFO, RoleId(roleid));
	rpc->userid = userid;
	rpc->source = GetRoleInfo::SOURCE_REMOTE; 
	rpc->save_zoneid = zoneid;
	
	if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
		CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid, GetRemoteRoleInfo_Re(ERR_COMMUNICATION, roleid, remote_roleid, userid, GRoleInfo()));
	}
}

void CrossGuardServer::Initialize()
{
	if(_init) return;
	LOG_TRACE("CrossGuardServer Init!");

	Timer::Attach(this);

	_init = true;
}

void CrossGuardServer::Register(CARNIVAL_TYPE key, int day, int begtime, int endtime, int* zlist, int zcount)
{
	_open_date_map[key].push_back(data_node(day,begtime,endtime,zlist,zcount));
}

void CrossGuardServer::Update()
{
#define CGS_CHECK_INTERVAL 30

	if((++_tick)%CGS_CHECK_INTERVAL) 
		return;
	
	time_t now = Timer::GetTime();

	struct tm dt;
	localtime_r(&now, &dt);
	int second_of_day = _debug_second == -1 ? (dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec) : _debug_second;
	int wday = _debug_day == -1 ? dt.tm_wday : _debug_day;		

	typedef std::map<int/*zid*/,IntVector/*carnivalid*/ > ZOSMap;
	ZOSMap	zone_open_status;

	const CentralDeliveryServer::DSMap& dsmap = CentralDeliveryServer::GetInstance()->GetDsMap();
	CentralDeliveryServer::DSMap::const_iterator izlbeg = dsmap.begin();
	CentralDeliveryServer::DSMap::const_iterator izlend = dsmap.end();
	for(;izlbeg != izlend; ++izlbeg)
	{
		zone_open_status[izlbeg->first] = IntVector();
	}

	DateMap::iterator iter = _open_date_map.begin();
	DateMap::iterator iend = _open_date_map.end();
	
	for(;iter != iend;++iter)
	{
		DateList::iterator ilt = iter->second.begin();
		DateList::iterator ile = iter->second.end();
		
		for(; ilt != ile ; ++ilt)
		{
			if(ilt->check(wday,second_of_day))
			{
				if(ilt->zonelist.size() == 1 && ilt->zonelist[0] == -1) // all zone
				{
					ZOSMap::iterator zoiter = zone_open_status.begin();		
					ZOSMap::iterator zoiend = zone_open_status.end();		
					
					for(;zoiter != zoiend; ++ zoiter)
					{
						zoiter->second.push_back(iter->first);
					}
				}
				else
				{					 
					for(size_t i = 0; i < ilt->zonelist.size(); ++i )
					{
						zone_open_status[ilt->zonelist[i]].push_back(iter->first);
					}
				}
			}
		}
	}

	ZOSMap::iterator zoiter = zone_open_status.begin();		
	ZOSMap::iterator zoiend = zone_open_status.end();		
	
	for(;zoiter != zoiend; ++ zoiter)
	{
		CrossGuardNotify pro(zoiter->second);
		CentralDeliveryServer::GetInstance()->DispatchProtocol(zoiter->first,pro);
	}
}

void CrossGuardClient::Initialize()
{
	LOG_TRACE("CrossGuardClient Init!");
	_init = true;
}

void CrossGuardClient::OnUpdate(IntVector& clist)
{
	if(_init)
	{
		int openflag[CT_TYPE_END] = {0};
		for(size_t n = 0; n < clist.size(); ++n)
		{
			int k = clist[n];
			if(k >= CT_TYPE_BEG && k < CT_TYPE_END )
			{
				openflag[k] = 1;
			}
		}

		for(int key = CT_TYPE_BEG; key < CT_TYPE_END; ++key)
		{
			UniqueDataServer::GetInstance()->ModifyByDelivery(key+CARNIVAL_DOOR_UNKEY_BEG,openflag[key]);
		}
	}
}

bool CrossGuardClient::CanCross()
{
	if(!_init) return false;
	if(_debug_pass) return true;
	
	for(int key = UNCK_BEG; key < UNCK_END; ++key)
	{
		if(UniqueDataServer::GetInstance()->GetIntByDelivery(key) > 0)
			return true;
	}
	
	return false;
}

void CrossGuardClient::OnPlayerCross(int roleid,short type,int64_t mnfid,bool backflag)
{
	if(type < CT_TYPE_BEG || type >= CT_TYPE_END)
	{
		Log::log(LOG_ERR, "CrossGuardClient:OnPlayerCross role-%d type-%d flag-%d Err !", roleid,type,backflag?1:0);
		return;
	}

	if(!_init) // 允许跨回防止计数错误 , Gm或者改过配置
	{
		Log::log(LOG_WARNING,"CrossGuardClient:OnPlayerCross role-%d type-%d flag-%d Warn !", roleid,type,backflag?1:0);
	}

	int key = type + CARNIVAL_COUNT_UNKEY_BEG;
	int count = UniqueDataServer::GetInstance()->GetIntByDelivery(key);
	if(count < 0) count = 0;

	if(backflag) 
		--count;
	else
		++count;

	UniqueDataServer::GetInstance()->ModifyByDelivery(key,count);

	switch(type)
	{
		case CT_MNFACTION_BATTLE:
			//onplayercross...//todo guoshi
			CDC_MNFactionBattleMan::GetInstance()->OnMNFactionPlayerCross(mnfid, roleid, backflag);
			break;
	}
}

void CrossChatServer::OnRecv(const CrossChat& msg)
{
	if(!_init) return;
	
	WorldChat chat;
	chat.emotion = msg.emotion;
	chat.channel = msg.channel;
	chat.roleid  = msg.zoneid;
	chat.name = msg.name;
	chat.msg  = msg.msg;
	chat.data = msg.data;
	LinkServer::GetInstance().BroadcastProtocol(&chat);
	
	CrossChat_Re proto(msg.sn,msg.zoneid,msg.channel,msg.emotion,
					   msg.zoneid, msg.name,msg.msg,msg.data);
	
	CentralDeliveryServer::GetInstance()->BroadcastProtocol(&proto);
}

void CrossChatServer::OnSend(int roleid,unsigned char channel,unsigned char emotion,const Octets& name,const Octets& msg,const Octets& data,int zoneid)
{
	if(!_init)
	{
		Log::log(LOG_ERR, "CrossChatServer:OnSend role%d Err0 !", roleid);
		return;
	}
	
	CrossChat_Re proto(0,GDeliveryServer::GetInstance()->GetZoneid(),
						channel, emotion, roleid, name, msg, data);
	
	if(zoneid > 0)
		CentralDeliveryServer::GetInstance()->DispatchProtocol(zoneid,&proto);
	else
		CentralDeliveryServer::GetInstance()->BroadcastProtocol(&proto);
}

void CrossChatServer::Initialize()
{
	_init = true;
	LOG_TRACE("CrossChatServer Init!");
}

void CrossChatClient::OnSend(int roleid,unsigned char channel,unsigned char emotion,const Octets& name,const Octets& msg,const Octets& data)
{
	if(!_init)
	{
		Log::log(LOG_ERR, "CrossChatClient:OnSend role%d Err0 !", roleid);
		return;
	}
	CrossChat* proto = new CrossChat(++_sn,GDeliveryServer::GetInstance()->GetZoneid(),
			channel,emotion,roleid,Timer::GetTime(),name,msg,data);

	if(!proto)
	{
		Log::log(LOG_ERR, "CrossChatClient:OnSend role%d Err1 !", roleid);
		return;
	}

	_cache_map.insert(std::make_pair(proto->sn,proto));

	CentralDeliveryClient::GetInstance()->SendProtocol(proto);
}

void CrossChatClient::OnRecv(const CrossChat_Re& msg)
{
	LOG_TRACE("ChatBroadCast: sn=%d,zoneid=%d,challel=%d roleid=%d emotion=%d", msg.sn, 
			msg.zoneid, msg.channel, msg.roleid, msg.emotion);

	if(msg.zoneid == GDeliveryServer::GetInstance()->GetZoneid())
	{
		MSG_CACHE_MAP::iterator iter = _cache_map.find(msg.sn);
		if(iter != _cache_map.end())
		{
			if(iter->second) delete iter->second;
			_cache_map.erase(iter);	
		}
	}
	else
	{
		WorldChat chat;
		chat.emotion = msg.emotion;
		chat.channel = msg.channel;
		chat.roleid  = msg.roleid;
		chat.name = msg.name;
		chat.msg  = msg.msg;
		chat.data = msg.data;
		LinkServer::GetInstance().BroadcastProtocol(&chat);
	}
}

void CrossChatClient::Initialize()
{
	if(_init) return;
	LOG_TRACE("CrossChatClient Init!");

#define CCC_CHECK_INTERVAL 30
	IntervalTimer::Attach(this,CCC_CHECK_INTERVAL*1000000/IntervalTimer::Resolution());

	_init = true;
}

bool CrossChatClient::Update()
{
	if(!_init)
		return false;
	if(!CentralDeliveryClient::GetInstance()->IsConnect())
		return true;
	if(_cache_map.empty())
		return true;

	int now = Timer::GetTime();

	for(MSG_CACHE_MAP::iterator iter = _cache_map.begin(),iend = _cache_map.end();
		iter != iend; ++iter)
	{
		CrossChat* proto = iter->second;

		if(now - proto->timestamp > CCC_CHECK_INTERVAL)
			CentralDeliveryClient::GetInstance()->SendProtocol(proto);
	}
	return true;
}

void CrossSoloRankServer::Initialize()
{
	_init = true;
	LOG_TRACE("CrossSoloRankServer Init!");
}

void CrossSoloRankServer::OnRecv(const CrossSoloChallengeRank& msg)
{
	if(!_init) return;
	LOG_TRACE("CrossSoloRankServer OnRecv Zone-%d CrossSoloChallengeRank!",msg.data_ext.zoneid);
	// 插入中央服排行榜数据
	SoloChallengeRank::GetInstance().OnLoadLocalRank(msg);
	CrossSoloChallengeRank_Re proto(0, GDeliveryServer::GetInstance()->GetZoneid());
	CentralDeliveryServer::GetInstance()->DispatchProtocol(msg.data_ext.zoneid,proto);
}

void CrossSoloRankServer::OnRecv(const CrossSoloChallengeRank_Re& msg)
{
	LOG_TRACE("CrossSoloRankServer OnRecv Zone-%d CrossSoloChallengeRank_Re!",msg.zoneid);
	// 返回中央服排行榜数据
	CrossSoloChallengeRank proto;
	SoloChallengeRank::GetInstance().OnSaveGlobalRank(proto);
	CentralDeliveryServer::GetInstance()->DispatchProtocol(msg.zoneid,proto);
}

void CrossSoloRankClient::Initialize()
{
	if(_init) return;
	LOG_TRACE("CrossSoloRankClient Init!");

#define CSRC_CHECK_INTERVAL 300
	IntervalTimer::Attach(this,CSRC_CHECK_INTERVAL*1000000/IntervalTimer::Resolution());

	_init = true;
}

void CrossSoloRankClient::OnRecv(const CrossSoloChallengeRank& msg)
{
	_state = CSR_NORMAL;
	LOG_TRACE("CrossSoloRankClient OnRecv Zone-%d CrossSoloChallengeRank!",msg.data_ext.zoneid);
	// 更新普通服全服排行榜数据
	SoloChallengeRank::GetInstance().OnLoadGlobalRank(msg);
}

void CrossSoloRankClient::OnRecv(const CrossSoloChallengeRank_Re& msg)
{
	_state = CSR_SYNC_RECV;
	LOG_TRACE("CrossSoloRankClient OnRecv Zone-%d CrossSoloChallengeRank_Re!",msg.zoneid);
}

bool CrossSoloRankClient::Update()
{
#define CSR_SYNC_HOUR 6
#define CSR_SYNC_HALF_MIN 30

	if(!_init)
		return false;

	time_t now = Timer::GetTime();
	struct tm dt;
	localtime_r(&now, &dt);

	switch(_state)
	{
		case CSR_UNINIT:
			{
				// 向中央服请求数据
				CrossSoloChallengeRank_Re proto(0, GDeliveryServer::GetInstance()->GetZoneid());
				CentralDeliveryClient::GetInstance()->SendProtocol(proto);

                LOG_TRACE("CrossSoloRankClient Update CSR_UNINIT, send CrossSoloChallengeRank_Re to server, hour=%d, min=%d, state=%d.",
                    dt.tm_hour, dt.tm_min, _state);
			}
			break;
		case CSR_NORMAL:
			{
				// 检查是否6:00 后切换到同步状态
				if(dt.tm_hour == CSR_SYNC_HOUR)
				{
					_state = CSR_SYNC_SEND;	
				}

                LOG_TRACE("CrossSoloRankClient Update CSR_NORMAL, hour=%d, min=%d, state=%d.",
                    dt.tm_hour, dt.tm_min, _state);
			}
			break;
		case CSR_SYNC_SEND:
			{
				// 同步到中央服 检查是否6:30后输出错误，但保证本服数据能同步到中心服
				CrossSoloChallengeRank proto;
				SoloChallengeRank::GetInstance().OnSaveLocalRank(proto); 
				CentralDeliveryClient::GetInstance()->SendProtocol(proto);

				if(dt.tm_hour != CSR_SYNC_HOUR || dt.tm_min >= CSR_SYNC_HALF_MIN)
				{
					Log::log(LOG_ERR, "CrossSoloRankClient Sync Timeout");
				}

                LOG_TRACE("CrossSoloRankClient Update CSR_SYNC_SEND, send CrossSoloChallengeRank to server, hour=%d, min=%d, state=%d.",
                    dt.tm_hour, dt.tm_min, _state);
			}
			break;
		case CSR_SYNC_RECV:
			{
				// 非6:00~6:30间都从中央服拖全服排行数据
				if(dt.tm_hour != CSR_SYNC_HOUR || dt.tm_min >= CSR_SYNC_HALF_MIN)
				{
					CrossSoloChallengeRank_Re proto(0, GDeliveryServer::GetInstance()->GetZoneid());
					CentralDeliveryClient::GetInstance()->SendProtocol(proto);

                    LOG_TRACE("CrossSoloRankClient Update CSR_SYNC_RECV, send CrossSoloChallengeRank_Re to server, hour=%d, min=%d, state=%d.",
                        dt.tm_hour, dt.tm_min, _state);
                }
			}
			break;
	}

	return true;
}

}

