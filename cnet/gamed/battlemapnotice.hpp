
#ifndef __GNET_BATTLEMAPNOTICE_HPP
#define __GNET_BATTLEMAPNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcity"
#include "gsp_if.h"


namespace GNET
{

class BattleMapNotice : public GNET::Protocol
{
	#include "battlemapnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		for(size_t j=0; j < cities.size(); j ++)
		{
			GMSV::SetCityOwner(cities[j].id, cities[j].level, cities[j].owner);
		}
		GMSV::RefreshCityBonus();
	}
};

};

#endif
