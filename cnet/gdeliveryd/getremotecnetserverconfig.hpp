
#ifndef __GNET_GETREMOTECNETSERVERCONFIG_HPP
#define __GNET_GETREMOTECNETSERVERCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "getremotecnetserverconfig_re.hpp"

namespace GNET
{

class GetRemoteCNetServerConfig : public GNET::Protocol
{
	#include "getremotecnetserverconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("Recv GetRemoteCNetServerConfig key_size=%d, linksid=%d, localsid=%d", keys.size(), linksid, localsid);
		
		GetRemoteCNetServerConfig_Re re;
		re.linksid = linksid;
		re.localsid = localsid;

		for(unsigned int i = 0; i < keys.size(); ++i) {
			switch(keys[i])
			{
				case CNET_CONFIG_COUNTRYBATTLE_BONUS:
				{
					bool is_open = !DisabledSystem::GetDisabled(SYS_COUNTRYBATTLE);
					if(is_open) { //¹úÕ½¿ªÆô
						int group = CentralDeliveryServer::GetInstance()->GetGroupIdByZoneId(zoneid); 
						int total_bonus = CountryBattleMan::GetDefault(group)->GetTotalBonus();
						re.result.push_back(IntOctets(keys[i], Marshal::OctetsStream() << total_bonus));
					} else {
						re.result.push_back(IntOctets(keys[i], Marshal::OctetsStream() << -1));
					}
					
					break;
				}
			}
		}

		if(re.result.size() > 0) manager->Send(sid, re);
	}
};

};

#endif
