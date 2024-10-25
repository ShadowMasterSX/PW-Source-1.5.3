
#ifndef __GNET_COUNTRYBATTLEEND_HPP
#define __GNET_COUNTRYBATTLEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrybattlepersonalscore"

namespace GNET
{

class CountryBattleEnd : public GNET::Protocol
{
	#include "countrybattleend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
