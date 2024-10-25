
#ifndef __GNET_COUNTRYBATTLEOFFLINENOTICE_HPP
#define __GNET_COUNTRYBATTLEOFFLINENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleOfflineNotice : public GNET::Protocol
{
	#include "countrybattleofflinenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
