
#ifndef __GNET_GAME2AU_HPP
#define __GNET_GAME2AU_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class Game2AU : public GNET::Protocol
{
	#include "game2au"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
