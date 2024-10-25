
#ifndef __GNET_REMOVEROLE_HPP
#define __GNET_REMOVEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RemoveRole : public GNET::Protocol
{
	#include "removerole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
