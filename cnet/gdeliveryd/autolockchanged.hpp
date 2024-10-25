
#ifndef __GNET_AUTOLOCKCHANGED_HPP
#define __GNET_AUTOLOCKCHANGED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutolockChanged : public GNET::Protocol
{
	#include "autolockchanged"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
