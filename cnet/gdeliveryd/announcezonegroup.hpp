
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
		int group_index = -1;
		int zoneid = GDeliveryServer::GetInstance()->GetZoneid();
		if(accepted_zone_list.find(zoneid) != accepted_zone_list.end())
		{
			group_index = accepted_zone_list[zoneid];
		}	

		if(is_central)
			CentralDeliveryServer::GetInstance()->SetAcceptedZoneGroup(accepted_zone_list);
		if(!DisabledSystem::GetDisabled(SYS_COUNTRYBATTLE))
			CountryBattleMan::OnInitialize(group_index, group, is_central);

		if(!DisabledSystem::GetDisabled(SYS_MNFACTIONBATTLE))
		{
			if(is_central)
			{
				CDS_MNFactionBattleMan::GetInstance()->OnInitialize();
			}
			else
			{
				CDC_MNFactionBattleMan::GetInstance()->OnInitialize();
			}
		}
	}
};

};

#endif
