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
				UserContainer::GetInstance().UserLogout(pinfo);
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
				UserContainer::GetInstance().UserLogout(pinfo);
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

}

