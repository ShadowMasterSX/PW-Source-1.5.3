
#ifndef __GNET_ROLELIST_HPP
#define __GNET_ROLELIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class RoleList : public GNET::Protocol
{
	#include "rolelist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidUser(sid,userid)) {
			DEBUG_PRINT("LinkServer::RoleList::Active Close Session %d,(userid=%d)\n",sid,userid);
			lsm->Close(sid);
			lsm->ActiveCloseSession(sid);
			return;
		}
		this->localsid=sid;
		if ( GDeliveryClient::GetInstance()->SendProtocol(this) )
		{
			lsm->ChangeState(sid,&state_GRoleListReceive);
		}
		else
			lsm->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
