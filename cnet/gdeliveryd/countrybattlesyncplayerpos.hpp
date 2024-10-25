
#ifndef __GNET_COUNTRYBATTLESYNCPLAYERPOS_HPP
#define __GNET_COUNTRYBATTLESYNCPLAYERPOS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleSyncPlayerPos : public GNET::Protocol
{
	#include "countrybattlesyncplayerpos"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
