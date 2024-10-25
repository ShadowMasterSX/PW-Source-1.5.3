
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
		GameDBManager* gdbm = GameDBManager::GetInstance();
		if(is_central != (char)gdbm->IsCentralDB())
		{
			Log::log(LOG_ERR, "CrossRelated AnnounceCentralDelivery, delivery type %d does not match DB type, disconnect", is_central);
			manager->Close(sid);
			return;
		}
		else if(is_central)
		{
			std::vector<int>& zones = accepted_zone_list;
			if(!gdbm->IsAcceptedZoneListMatch(zones))
			{
				Log::log(LOG_ERR, "CrossRelated AnnounceCentralDelivery, accepted zoneid list is not match, please check the gamesys.conf, disconnect");
				manager->Close(sid);
				return;
			}
		}
	}
};

};

#endif
