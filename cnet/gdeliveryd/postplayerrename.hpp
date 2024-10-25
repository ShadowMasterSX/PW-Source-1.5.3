
#ifndef __GNET_POSTPLAYERRENAME_HPP
#define __GNET_POSTPLAYERRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PostPlayerRename : public GNET::Protocol
{
	#include "postplayerrename"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
