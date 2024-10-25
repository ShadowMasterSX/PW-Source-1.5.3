
#ifndef __GNET_COUNTRYBATTLEENTER_HPP
#define __GNET_COUNTRYBATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleEnter : public GNET::Protocol
{
	#include "countrybattleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
