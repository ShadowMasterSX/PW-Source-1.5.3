
#ifndef __GNET_ANNOUNCECENTRALDELIVERY_HPP
#define __GNET_ANNOUNCECENTRALDELIVERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceCentralDelivery : public GNET::Protocol
{
	#include "announcecentraldelivery"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
