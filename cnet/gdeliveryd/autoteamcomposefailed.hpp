
#ifndef __GNET_AUTOTEAMCOMPOSEFAILED_HPP
#define __GNET_AUTOTEAMCOMPOSEFAILED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutoTeamComposeFailed : public GNET::Protocol
{
	#include "autoteamcomposefailed"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
