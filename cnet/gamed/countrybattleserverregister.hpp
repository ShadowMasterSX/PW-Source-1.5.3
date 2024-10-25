
#ifndef __GNET_COUNTRYBATTLESERVERREGISTER_HPP
#define __GNET_COUNTRYBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleServerRegister : public GNET::Protocol
{
	#include "countrybattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
