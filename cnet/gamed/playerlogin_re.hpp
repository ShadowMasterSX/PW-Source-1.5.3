
#ifndef __GNET_PLAYERLOGIN_RE_HPP
#define __GNET_PLAYERLOGIN_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerLogin_Re : public GNET::Protocol
{
	#include "playerlogin_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
