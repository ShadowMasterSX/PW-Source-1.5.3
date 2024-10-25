
#ifndef __GNET_ACACCUSERE_HPP
#define __GNET_ACACCUSERE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ACAccuseRe : public GNET::Protocol
{
	#include "acaccusere"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
