
#ifndef __GNET_KEYEXCHANGE_HPP
#define __GNET_KEYEXCHANGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class KeyExchange : public GNET::Protocol
{
	#include "keyexchange"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
