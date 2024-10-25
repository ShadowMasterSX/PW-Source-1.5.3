
#ifndef __GNET_POSTFACTIONRENAME_HPP
#define __GNET_POSTFACTIONRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PostFactionRename : public GNET::Protocol
{
	#include "postfactionrename"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
