
#ifndef __GNET_ANNOUNCECENTRALDELIVERY_HPP
#define __GNET_ANNOUNCECENTRALDELIVERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "announcezonegroup.hpp"

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

/*			std::vector<int>& zones = accepted_zone_list;
			if(!gdbm->IsAcceptedZoneListMatch(zones))
			{
				Log::log(LOG_ERR, "CrossRelated AnnounceCentralDelivery, accepted zoneid list is not match, please check the gamesys.conf, disconnect");
				manager->Close(sid);
				return;
			}
*/
		AnnounceZoneGroup proto;
		proto.is_central = is_central;
		gdbm->GetAcceptedZoneList(proto.accepted_zone_list,proto.group);
		GameDBServer::GetInstance()->Send2Delivery(proto);		
	}
};

};

#endif
