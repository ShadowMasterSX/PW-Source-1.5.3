
#ifndef __GNET_SELECTROLE_HPP
#define __GNET_SELECTROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "gproviderserver.hpp"
#include "playerlogin.hpp"
namespace GNET
{

class SelectRole : public GNET::Protocol
{
	#include "selectrole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		PlayerLogin pro;
		pro.roleid = roleid;
		pro.provider_link_id = GProviderServer::GetProviderServerID();
		pro.localsid = sid;
		
		//PlayerLogin协议最后的flag含义 0正常登陆 1原服->跨服 2跨服->原服 3直接登录跨服
		pro.flag = flag;
		
		if(GDeliveryClient::GetInstance()->SendProtocol(pro)) {
			lsm->ChangeState(sid,&state_GSelectRoleReceive);
		} else {
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
		}
	}
};

};

#endif
