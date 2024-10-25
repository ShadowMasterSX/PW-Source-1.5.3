
#ifndef __GNET_S2CMULTICAST_HPP
#define __GNET_S2CMULTICAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "player"
namespace GNET
{

class S2CMulticast : public GNET::Protocol
{
	#include "s2cmulticast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
