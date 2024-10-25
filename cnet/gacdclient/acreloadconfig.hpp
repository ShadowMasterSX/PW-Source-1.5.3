
#ifndef __GNET_ACRELOADCONFIG_HPP
#define __GNET_ACRELOADCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACReloadConfig : public GNET::Protocol
{
	#include "acreloadconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
