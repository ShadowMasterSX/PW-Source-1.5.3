
#ifndef __GNET_PLAYERREQUITEFRIEND_HPP
#define __GNET_PLAYERREQUITEFRIEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerRequiteFriend : public GNET::Protocol
{
	#include "playerrequitefriend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
