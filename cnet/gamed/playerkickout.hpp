
#ifndef __GNET_PLAYERKICKOUT_HPP
#define __GNET_PLAYERKICKOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE
	#include "playerkickout_re.hpp"
#endif

#include "gproviderclient.hpp"
void user_kickout(int cs_index,int sid,int uid);
namespace GNET
{

class PlayerKickout : public GNET::Protocol
{
	#include "playerkickout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
#ifdef _WAIT_TIME
		sleep(5);
#endif
		DEBUG_PRINT ("gamed(%d):: receive playerkickout, roleid=%d,link_id=%d,localsid=%d\n",GProviderClient::GetGameServerID(),roleid,provider_link_id,localsid);
		
		manager->Send(sid,PlayerKickout_Re(ERR_SUCCESS,roleid));
		//manager->Send(sid,PlayerKickout_Re(ERR_LOGINFAIL,roleid));
			
#endif
		user_kickout(provider_link_id,localsid,roleid);
	}
};

};

#endif
