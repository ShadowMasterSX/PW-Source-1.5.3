
#ifndef __GNET_COUNTRYBATTLELEAVENOTICE_HPP
#define __GNET_COUNTRYBATTLELEAVENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleLeaveNotice : public GNET::Protocol
{
	#include "countrybattleleavenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
