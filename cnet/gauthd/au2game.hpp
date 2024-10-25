
#ifndef __GNET_AU2GAME_HPP
#define __GNET_AU2GAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AU2Game : public GNET::Protocol
{
	#include "au2game"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
