
#ifndef __GNET_ANNOUNCEGM_HPP
#define __GNET_ANNOUNCEGM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class AnnounceGM : public GNET::Protocol
{
	#include "announcegm"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
