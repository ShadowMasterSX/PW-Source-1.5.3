
#ifndef __GNET_AUTOTEAMCONFIGREGISTER_HPP
#define __GNET_AUTOTEAMCONFIGREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "autoteamconfigdata"

namespace GNET
{

class AutoTeamConfigRegister : public GNET::Protocol
{
	#include "autoteamconfigregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
