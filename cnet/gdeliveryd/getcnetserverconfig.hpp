
#ifndef __GNET_GETCNETSERVERCONFIG_HPP
#define __GNET_GETCNETSERVERCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "getremotecnetserverconfig.hpp"

namespace GNET
{

class GetCNetServerConfig : public GNET::Protocol
{
	#include "getcnetserverconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("Recv GetCNetServerConfig key_size=%d, roleid=%d", keys.size(), roleid);

		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(pInfo == NULL) return;
		
		bool need_get_remote_config = false;
		IntVector remote_keys;
		
		GetCNetServerConfig_Re re;
		re.localsid = pInfo->localsid;
		
		for(unsigned int i = 0; i < keys.size(); ++i) {
			switch(keys[i])
			{
				case CNET_CONFIG_COUNTRYBATTLE_BONUS:
				{
					bool is_open = !DisabledSystem::GetDisabled(SYS_COUNTRYBATTLE);
					if(is_open) { //国战开启
						int total_bonus = CountryBattleMan::GetDefault()->GetTotalBonus();
						re.result.push_back(IntOctets(keys[i], Marshal::OctetsStream() << total_bonus));
					} else { //国战未开
						bool is_central = ((GDeliveryServer*)manager)->IsCentralDS();
						if(!is_central && CentralDeliveryClient::GetInstance()->IsConnect()) { //连接了中央服的普通服
							need_get_remote_config = true;
							remote_keys.push_back(keys[i]);
						} else {
							re.result.push_back(IntOctets(keys[i], Marshal::OctetsStream() << -1));
						}
					}
					
					break;
				}
			}
		}

		if(re.result.size() > 0) manager->Send(sid, re);
		
		if(need_get_remote_config) {
			GetRemoteCNetServerConfig pro(remote_keys, pInfo->linksid, pInfo->localsid, GDeliveryServer::GetInstance()->GetZoneid());
			CentralDeliveryClient::GetInstance()->SendProtocol(pro);
		}
	}
};

};

#endif
