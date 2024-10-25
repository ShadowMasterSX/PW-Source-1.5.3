
#ifndef __GNET_PLAYEROFFLINE_HPP
#define __GNET_PLAYEROFFLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE
#include "playeroffline_re.hpp"
#endif
#include "gproviderclient.hpp"
void user_lost_connection(int cs_index,int sid,int uid);
namespace GNET
{

class PlayerOffline : public GNET::Protocol
{
	#include "playeroffline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
#ifdef _WAIT_TIME
		sleep(5);
#endif
		DEBUG_PRINT ("gamed(%d):: receive playeroffline, roleid=%d,link_id=%d,localsid=%d\n",GProviderClient::GetGameServerID(),roleid,provider_link_id,localsid);
		
		manager->Send(sid,PlayerOffline_Re(ERR_SUCCESS,roleid));
		//manager->Send(sid,PlayerOffline_Re(ERR_LOGINFAIL,roleid));
#endif
		user_lost_connection(provider_link_id,localsid,roleid);
	}
};

};

#endif
