
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
		if(is_central != (char)GFactionServer::GetInstance()->IsCentralFaction())
		{
			Log::log(LOG_ERR, "CrossRelated AnnounceCentralDelivery, delivery type %d does not match Faction type, disconnect", is_central);
			manager->Close(sid);
			return;
		}
	}
};

};

#endif
