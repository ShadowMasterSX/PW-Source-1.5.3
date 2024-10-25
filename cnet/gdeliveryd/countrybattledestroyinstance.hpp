
#ifndef __GNET_COUNTRYBATTLEDESTROYINSTANCE_HPP
#define __GNET_COUNTRYBATTLEDESTROYINSTANCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleDestroyInstance : public GNET::Protocol
{
	#include "countrybattledestroyinstance"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
