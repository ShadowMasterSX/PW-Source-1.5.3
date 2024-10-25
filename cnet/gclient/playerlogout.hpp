
#ifndef __GNET_PLAYERLOGOUT_HPP
#define __GNET_PLAYERLOGOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE
	//#include "makechoice.h"
	#include "rolelist.hpp"
	#include "glinkclient.hpp"
#endif
namespace GNET
{

class PlayerLogout : public GNET::Protocol
{
	#include "playerlogout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		DEBUG_PRINT("client: receive playerlogout,retcode = %d\n.",result);
		if (result	== _PLAYER_LOGOUT_FULL)
		{
			DEBUG_PRINT("logout full. disconnect.\n");
			manager->Close(sid);
		}
		else if (result == _PLAYER_LOGOUT_HALF)
		{
			DEBUG_PRINT("logout half. reselect role.\n");
			manager->ChangeState(sid,&state_GSelectRoleClient);
		//	MakeChoice(ROLEID2USERID(roleid),manager,sid);
			((GLinkClient*)manager)->SendProtocol(RoleList(((GLinkClient*)manager)->userid,_SID_INVALID,_HANDLE_BEGIN));
		}
#endif
	}
};

};

#endif
