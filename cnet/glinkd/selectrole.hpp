
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
		
		//PlayerLoginЭ������flag���� 0������½ 1ԭ��->��� 2���->ԭ�� 3ֱ�ӵ�¼���
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
