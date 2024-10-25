
#ifndef __GNET_STATUSANNOUNCE_HPP
#define __GNET_STATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gauthserver.hpp"
namespace GNET
{

class StatusAnnounce : public GNET::Protocol
{
	#include "statusannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GAuthServer* ausm=GAuthServer::GetInstance();
		Thread::RWLock::WRScoped l(ausm->locker_map);
		GAuthServer::UserMap::const_iterator it=ausm->usermap.find(userid);
		if (it!=ausm->usermap.end() && (*it).second.sid==sid)
		{
			ausm->usermap.erase(userid);
			DEBUG_PRINT("gauthd:: user %d offline. online user count is %d\n",userid,ausm->usermap.size());
		}
	}
};

};

#endif
