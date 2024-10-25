
#ifndef __GNET_COUNTRYBATTLEJOINNOTICE_HPP
#define __GNET_COUNTRYBATTLEJOINNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleJoinNotice : public GNET::Protocol
{
	#include "countrybattlejoinnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
