
#ifndef __GNET_ACREMOTEEXE_HPP
#define __GNET_ACREMOTEEXE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ACRemoteExe : public GNET::Protocol
{
	#include "acremoteexe"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
