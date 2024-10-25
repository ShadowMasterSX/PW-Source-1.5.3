
#ifndef __GNET_COUNTRYBATTLECONFIGNOTIFY_HPP
#define __GNET_COUNTRYBATTLECONFIGNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrycapital"

namespace GNET
{

class CountryBattleConfigNotify : public GNET::Protocol
{
	#include "countrybattleconfignotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
