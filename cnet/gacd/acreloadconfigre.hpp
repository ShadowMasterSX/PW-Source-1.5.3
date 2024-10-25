
#ifndef __GNET_ACRELOADCONFIGRE_HPP
#define __GNET_ACRELOADCONFIGRE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACReloadConfigRe : public GNET::Protocol
{
	#include "acreloadconfigre"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
