
#ifndef __GNET_BATTLEFACTIONNOTICE_HPP
#define __GNET_BATTLEFACTIONNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BattleFactionNotice : public GNET::Protocol
{
	#include "battlefactionnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
