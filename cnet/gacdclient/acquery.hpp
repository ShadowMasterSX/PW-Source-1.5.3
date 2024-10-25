
#ifndef __GNET_ACQUERY_HPP
#define __GNET_ACQUERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACQuery : public GNET::Protocol
{
	#include "acquery"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
