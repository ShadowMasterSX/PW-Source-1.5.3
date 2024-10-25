
#ifndef __GNET_DISCONNECTPLAYER_HPP
#define __GNET_DISCONNECTPLAYER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class DisconnectPlayer : public GNET::Protocol
{
	#include "disconnectplayer"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		GLinkServer::RoleMap::const_iterator it=lsm->roleinfomap.find(roleid);
		if (it==lsm->roleinfomap.end()) 
			return;
		if ( (*it).second.roleid!=roleid ) 
			return;
		localsid=(*it).second.sid;
		lsm->Close(localsid);
		lsm->ActiveCloseSession(localsid);
		DEBUG_PRINT("LinkServer::DisconnectPlayer:: Active Close session %d(roleid=%d)\n",localsid,roleid);
	}
};

};

#endif
