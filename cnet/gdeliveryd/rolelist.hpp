
#ifndef __GNET_ROLELIST_HPP
#define __GNET_ROLELIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "getuser.hrp"
#include "getroleinfo.hrp"
#include "rolelist_re.hpp"
#include "gdeliveryserver.hpp"

#include "gauthclient.hpp"
#include "accountingrequest.hpp"
#include "mapuser.h"
namespace GNET
{

class RoleList : public GNET::Protocol
{
	#include "rolelist"
	Manager* manager;
	Manager::Session::ID sid;
	void GetUserInfo(UserInfo* user,Manager::Session::ID sid)
	{
		GetUser* rpc=(GetUser*) Rpc::Call(RPC_GETUSER,UserArg(userid, Timer::GetTime(), user->ip));
		rpc->save_link_sid=sid;
		rpc->save_localsid=localsid;
		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	void GetNextRole(UserInfo* user,Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		int next_role = user->rolelist.GetNextRole();
		if (next_role==_HANDLE_END)
		{
			dsm->Send(sid,RoleList_Re(ERR_SUCCESS,_HANDLE_END,userid,localsid,RoleInfoVector()));
		}
		else
		{
			GetRoleInfo* rpc=(GetRoleInfo*) Rpc::Call(RPC_GETROLEINFO,RoleId(user->logicuid+next_role));
			rpc->userid = userid;
			rpc->source = GetRoleInfo::SOURCE_LOCAL;

			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo* userinfo = UserContainer::GetInstance().FindUser(userid);
		if (NULL==userinfo) 
			return; 
		
		//跨服不应该收到RoleList协议
		if(GDeliveryServer::GetInstance()->IsCentralDS()) {
			Log::log(LOG_ERR, "User %d try to Get RoleList from CentralDS, refuse him!", userid);
			return;
		}

		/* send request to game DB */
		if (!userinfo->rolelist.IsRoleListInitialed()) 
		{
			GetUserInfo(userinfo,sid);
		}
		else
		{
			GetNextRole(userinfo,sid);
		}
		return;
	}
};

};

#endif
