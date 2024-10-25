
#ifndef __GNET_COUNTRYBATTLECONFIGNOTIFY_HPP
#define __GNET_COUNTRYBATTLECONFIGNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrycapital"
#include "gsp_if.h"

void notify_country_battle_config(GMSV::CBConfig * config);

namespace GNET
{

class CountryBattleConfigNotify : public GNET::Protocol
{
	#include "countrybattleconfignotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GMSV::CBConfig * pConfig = (GMSV::CBConfig *)malloc(sizeof(GMSV::CBConfig) + country_capitals.size()*sizeof(GMSV::CBConfig::CountryCapital));
		if(pConfig == NULL) return;
		pConfig->capital_count = country_capitals.size();
		for(size_t i=0; i<pConfig->capital_count; i++)
		{
			pConfig->capital_list[i].country_id = country_capitals[i].country_id;
			pConfig->capital_list[i].worldtag   = country_capitals[i].worldtag;
			pConfig->capital_list[i].posx       = country_capitals[i].posx;
			pConfig->capital_list[i].posy       = country_capitals[i].posy;
			pConfig->capital_list[i].posz       = country_capitals[i].posz;
		}

		notify_country_battle_config(pConfig);

		free(pConfig);
	}
};

};

#endif
