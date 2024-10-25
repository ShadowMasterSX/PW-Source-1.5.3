
#ifndef __GNET_PLAYERINFOUPDATE_HPP
#define __GNET_PLAYERINFOUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerInfoUpdate : public GNET::Protocol
{
	#include "playerinfoupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
