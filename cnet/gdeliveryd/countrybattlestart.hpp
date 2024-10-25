
#ifndef __GNET_COUNTRYBATTLESTART_HPP
#define __GNET_COUNTRYBATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleStart : public GNET::Protocol
{
	#include "countrybattlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
