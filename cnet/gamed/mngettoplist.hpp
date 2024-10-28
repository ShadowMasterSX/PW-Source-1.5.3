
#ifndef __GNET_MNGETTOPLIST_HPP
#define __GNET_MNGETTOPLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetTopList : public GNET::Protocol
{
	#include "mngettoplist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
