
#ifndef __GNET_GAMEPOSTCANCEL_HPP
#define __GNET_GAMEPOSTCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GamePostCancel : public GNET::Protocol
{
	#include "gamepostcancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
