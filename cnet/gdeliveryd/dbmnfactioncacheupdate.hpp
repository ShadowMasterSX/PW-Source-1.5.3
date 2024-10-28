
#ifndef __GNET_DBMNFACTIONCACHEUPDATE_HPP
#define __GNET_DBMNFACTIONCACHEUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mndomaininfo"
#include "mnfactioninfo"

namespace GNET
{

class DBMNFactionCacheUpdate : public GNET::Protocol
{
	#include "dbmnfactioncacheupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
