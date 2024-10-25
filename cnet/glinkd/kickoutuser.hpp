
#ifndef __GNET_KICKOUTUSER_HPP
#define __GNET_KICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryclient.hpp"
#include "glinkserver.hpp"
namespace GNET
{

class KickoutUser : public GNET::Protocol
{
	#include "kickoutuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		DEBUG_PRINT("glinkd::receive kickoutuser from delivery.userid=%d,localsid=%d,cause=%d,[manager %p]\n",userid,localsid,cause,manager);
		if ( cause==0 ) //means let the users disconnect directly
		{
			lsm->Close(localsid);
			lsm->ActiveCloseSession(localsid);
			DEBUG_PRINT("glinkd:: AC kickoutuser. Active Close session. userid=%d,localsid=%d\n",userid,localsid);
			return;
		}
		if (lsm->ValidUser(localsid,userid))
		{
			switch (cause)
			{
			case ERR_KICKOUT:
				lsm->SessionError(localsid,cause,"Be kicked out.");
				break;
			case ERR_ROLELIST:
				lsm->SessionError(localsid,cause,"Get Role list failed.");
				break;
			case ERR_LOGINFAIL:
				lsm->SessionError(localsid,cause,"login failed.");
				break;
			case ERR_CREATEROLE:
				lsm->SessionError(localsid,cause,"create role failed.");
				break;
			case ERR_GENERAL:
				lsm->SessionError(localsid,cause,"general error.");
				break;
			default:
				lsm->SessionError(localsid,cause,"");
				break;
			}
		}
	}
};

};

#endif
