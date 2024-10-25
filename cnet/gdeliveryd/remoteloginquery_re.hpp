
#ifndef __GNET_REMOTELOGINQUERY_RE_HPP
#define __GNET_REMOTELOGINQUERY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "touchplayerdata.hrp"
#include "playerlogin_re.hpp"

namespace GNET
{

class RemoteLoginQuery_Re : public GNET::Protocol
{
	#include "remoteloginquery_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//由于原服和跨服的roleid和remote_roleid正好相反，所以受到该协议时，要交换二者的位置
		int tmp = remote_roleid;
		remote_roleid = roleid;
		roleid = tmp;

		LOG_TRACE("CrossRelated Recv RemoteLoginQuery_Re retcode %d roleid %d remote_roleid %d userid %d flag %d", retcode, roleid, remote_roleid, userid, flag);
		
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if (!dsm->IsCentralDS()) return;
		
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if (pinfo == NULL || pinfo->status != _STATUS_REMOTE_LOGINQUERY) {
			Log::log(LOG_ERR, "CrossRelated RemoteLoginQuery_Re userid %d status %d invalid", userid, pinfo == NULL ? 0 : pinfo->status);
			return;
		}
		
		//PlayerIdentityMatch 时 Push 的
		RemoteLoggingUsers::GetInstance().Pop(userid);
		if(retcode == ERR_SUCCESS && flag == DS_TO_CENTRALDS) {
			TouchPlayerData* rpc = (TouchPlayerData*)Rpc::Call(RPC_TOUCHPLAYERDATA, TouchPlayerDataArg(roleid, userid));
			rpc->flag = flag;

			if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
				Log::log(LOG_ERR, "CrossRelated RemoteLoginQuery_Re TouchPlayerData Failed, roleid %d, userid %d", roleid, userid);
				retcode = 105;
			} else {
				LOG_TRACE("CrossRelated Send to TouchPlayerData userid %d roleid %d", userid, roleid);
			}
		}
		
		if (retcode != ERR_SUCCESS || flag == DIRECT_TO_CENTRALDS) {
			PlayerLogin_Re re;

			re.result = retcode;
			re.roleid = roleid;
			re.src_provider_id = pinfo->gameid;
			re.localsid = pinfo->localsid;
			re.flag = flag;
			
			if (retcode == ERR_SUCCESS) {

				PlayerLogin_Re::RealLogin(roleid, pinfo);
				pinfo->status = _STATUS_READYGAME;
				
			} else {
				UserContainer::GetInstance().UserLogout(pinfo);
			}
			
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			dsm->Send(pinfo->linksid, re);
			dsm->BroadcastStatus();
			
			LOG_TRACE("CrossRelated Send PlayerLogin_Re to glink, userid %d roleid %d ret %d", userid, roleid, re.result);
		}
	}
};

};

#endif
