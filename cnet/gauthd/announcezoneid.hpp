
#ifndef __GNET_ANNOUNCEZONEID_HPP
#define __GNET_ANNOUNCEZONEID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gauthserver.hpp"
namespace GNET
{

class AnnounceZoneid : public GNET::Protocol
{
	#include "announcezoneid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::Mutex::Scoped l(GAuthServer::GetInstance()->locker_zonemap);
		GAuthServer::GetInstance()->zonemap[sid]=zoneid;
		DEBUG_PRINT("gauthd::annoucezoneid: zoneid=%d,aid=%d belongs to session %d\n",zoneid,aid,sid);
	}
};

};

#endif
