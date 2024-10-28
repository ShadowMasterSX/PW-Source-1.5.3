
#ifndef __GNET_GETENEMYLIST_HPP
#define __GNET_GETENEMYLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetEnemyList : public GNET::Protocol
{
	#include "getenemylist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
