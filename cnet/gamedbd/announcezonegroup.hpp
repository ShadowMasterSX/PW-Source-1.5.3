
#ifndef __GNET_ANNOUNCEZONEGROUP_HPP
#define __GNET_ANNOUNCEZONEGROUP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceZoneGroup : public GNET::Protocol
{
	#include "announcezonegroup"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
