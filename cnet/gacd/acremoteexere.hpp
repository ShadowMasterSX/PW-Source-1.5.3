
#ifndef __GNET_ACREMOTEEXERE_HPP
#define __GNET_ACREMOTEEXERE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACRemoteExeRe : public GNET::Protocol
{
	#include "acremoteexere"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
