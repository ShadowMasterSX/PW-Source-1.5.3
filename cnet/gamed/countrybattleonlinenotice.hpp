
#ifndef __GNET_COUNTRYBATTLEONLINENOTICE_HPP
#define __GNET_COUNTRYBATTLEONLINENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleOnlineNotice : public GNET::Protocol
{
	#include "countrybattleonlinenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
