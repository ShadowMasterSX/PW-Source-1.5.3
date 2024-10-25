
#ifndef __GNET_PLAYERASKFORPRESENT_HPP
#define __GNET_PLAYERASKFORPRESENT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerAskForPresent : public GNET::Protocol
{
	#include "playeraskforpresent"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
