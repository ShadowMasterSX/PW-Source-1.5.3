
#ifndef __GNET_PLAYERCHANGEDS_RE_HPP
#define __GNET_PLAYERCHANGEDS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerChangeDS_Re : public GNET::Protocol
{
	#include "playerchangeds_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//只有 retcode==ERR_SUCCESS 时 gdeliveryd 才会给glinkd发此协议
		GLinkServer* lsm = GLinkServer::GetInstance();
		
		if(flag == DS_TO_CENTRALDS || flag == CENTRALDS_TO_DS) {
			if(!lsm->ValidRole(localsid, roleid)) return;
			
			RoleData* uinfo = lsm->GetRoleInfo(roleid);
			if(uinfo != NULL) uinfo->status = _STATUS_ONLINE;
			
			//不删 roleinfo 因为此时可能客户端还会发送一些协议过来 即便服务器是直接忽略 不应该直接 close session. 客户端 close session 或者 glinkd 延时 30 秒 close session 时 roleinfo 会删掉
			//if(!lsm->RoleLogout( localsid, roleid )) return;
			 
		} else if(flag == DIRECT_TO_CENTRALDS) {
			if(!lsm->ValidUser(localsid, userid)) return;
		} else {
			return;
		}
		
		lsm->AccumulateSend(localsid, this);
		lsm->SetReadyCloseTime(localsid, 30);
		
		LOG_TRACE("CrossRelated Send PlayerChangeDS_Re to client ret %d roleid %d remote_roleid %d userid %d flag %d random.size %d dst_zoneid %d localsid %d",
				retcode, roleid, remote_roleid, userid, flag, random.size(), dst_zoneid, localsid);
		
		//将glink与gdeliveryd脱钩 使得客户端delsession的时候 不会通知gdeliveryd
		//此时deliveryd中Userinfo.status为REMOTE_HALFLOGIN状态 超时会自动清除
		SessionInfo* sinfo = lsm->GetSessionInfo(localsid);
		if(sinfo != NULL) {
			sinfo->userid = 0;
			
			time_t now = time(NULL);
			int status = 0x0;
			char strpeer[256];
			strcpy( strpeer,inet_ntoa(((struct sockaddr_in*)sinfo->GetPeer())->sin_addr) );
			
			Log::formatlog("logout","account=%.*s:userid=%d:sid=%d:peer=%s:time=%d:status=0x%x",
					sinfo->identity.size(), (char*)sinfo->identity.begin(), userid, localsid, strpeer, now-sinfo->login_time, status);
		}
	}
};

};

#endif
