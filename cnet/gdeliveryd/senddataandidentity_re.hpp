
#ifndef __GNET_SENDDATAANDIDENTITY_RE_HPP
#define __GNET_SENDDATAANDIDENTITY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "playerchangeds_re.hpp"
#include "activateplayerdata.hrp"
#include "delplayerdata.hrp"
#include "kickoutuser2.hpp"
#include "dbupdateplayercrossinfo.hrp"

namespace GNET
{

class SendDataAndIdentity_Re : public GNET::Protocol
{
	#include "senddataandidentity_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//由于原服和跨服的roleid和remote_roleid正好相反，所以受到该协议时，要交换二者的位置
		int tmp = remote_roleid;
		remote_roleid = roleid;
		roleid = tmp;

		LOG_TRACE("CrossRelated Recv SendDataAndIdentity_Re retcode %d roleid %d remote_roleid %d userid %d flag %d dst_zoneid %d", retcode, roleid, remote_roleid, userid, flag, dst_zoneid);
		
		if(flag == DS_TO_CENTRALDS || flag == DIRECT_TO_CENTRALDS) {
			if(GDeliveryServer::GetInstance()->IsCentralDS()) return;
		} else if (flag == CENTRALDS_TO_DS) {
			if(!GDeliveryServer::GetInstance()->IsCentralDS()) return;
		} else {
			return;
		}
		
		if(flag == DS_TO_CENTRALDS && retcode == ERR_SUCCESS && is_remote_roleid_changed) {
			//原服->跨服时，跨服roleid被重新分配
			DBUpdatePlayerCrossInfo* rpc = (DBUpdatePlayerCrossInfo*)Rpc::Call(RPC_DBUPDATEPLAYERCROSSINFO, DBUpdatePlayerCrossInfoArg(roleid, remote_roleid, userid, dst_zoneid));
			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if(pinfo == NULL || (pinfo->status != _STATUS_REMOTE_HALFLOGIN && pinfo->status != _STATUS_REMOTE_CACHERANDOM)) {
			Log::log(LOG_ERR, "CrossRelated SendDataAndIdentity_Re userid %d roleid %d remote_roleid %d ret %d user status %d invalid", 
				userid, roleid, remote_roleid, retcode, pinfo == NULL ? 0 : pinfo->status);
			return;
		}
		
		if(retcode != ERR_SUCCESS) {
			Log::log(LOG_ERR, "CrossRelated SendDataAndIdentity_Re roleid %d remote_roleid %d userid %d errno %d", roleid, remote_roleid, userid, retcode);
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, KickoutUser2(userid, pinfo->localsid, ERR_REMOTE_DATA_INVALID));
			UserContainer::GetInstance().UserLogout(pinfo);
			
			if(flag == DIRECT_TO_CENTRALDS && retcode == ERR_REMOTE_VERIFYFAILED) {
				LOG_TRACE("CrossRelated SendDataAndIdentity_Re Activate roleid %d data", roleid);
				GameDBClient::GetInstance()->SendProtocol((ActivatePlayerData *)Rpc::Call(RPC_ACTIVATEPLAYERDATA, ActivatePlayerDataArg(roleid)));
			} else {
				RemoteLoggingUsers::GetInstance().Pop(userid);
			}
			
			return;
		}
		
		CrossInfoData* pCrsRole = pinfo->GetCrossInfo(roleid);
		if(pCrsRole == NULL) return;
		if(pCrsRole->remote_roleid != remote_roleid) {
			pCrsRole->remote_roleid = remote_roleid;
		}

		PlayerChangeDS_Re re(ERR_SUCCESS, roleid, remote_roleid, userid, flag, pinfo->rand_key, dst_zoneid, pinfo->localsid, roleinfo_pack);
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, re);
		
		LOG_TRACE("CrossRelated Send PlayerChangeDS_Re to link, roleid %d remote_roleid %d userid %d flag %d rand_key.size %d dst_zoneid %d localsid %d", 
			re.roleid, re.remote_roleid, re.userid, re.flag, re.random.size(), re.dst_zoneid, pinfo->localsid);
		STAT_MIN5("LogoutNormal", 1);
		
		if(flag == CENTRALDS_TO_DS) {
			pinfo->src_zoneid = 0; //此处用了一个小捷径 目的是 UserLogout 时不向原服发 RemoteLogout. 因为此时原服 User 已经清除 (SendDataAndIdentity)
			UserContainer::GetInstance().UserLogout(pinfo); //pinfo 析构
			RemoteLoggingUsers::GetInstance().Pop(userid);
			
			DelPlayerData* rpc = (DelPlayerData*)Rpc::Call(RPC_DELPLAYERDATA, DelPlayerDataArg(roleid, userid));
			if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
				Log::log(LOG_ERR, "CrossRelated DelPlayerData send to DB error, roleid=%d", roleid, userid);
			} else {
				LOG_TRACE("CrossRelated Tell DB to delete player data roleid=%d", roleid);
			}
		}
	}
};

};

#endif
