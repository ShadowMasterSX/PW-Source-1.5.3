
#ifndef __GNET_PLAYERRENAME_HPP
#define __GNET_PLAYERRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerRename : public GNET::Protocol
{
	#include "playerrename"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
