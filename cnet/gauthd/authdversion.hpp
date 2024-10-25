
#ifndef __GNET_AUTHDVERSION_HPP
#define __GNET_AUTHDVERSION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AuthdVersion : public GNET::Protocol
{
	#include "authdversion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
