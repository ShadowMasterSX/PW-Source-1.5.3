
#ifndef __GNET_BATTLESTART_HPP
#define __GNET_BATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class BattleStart : public GNET::Protocol
{
	#include "battlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
