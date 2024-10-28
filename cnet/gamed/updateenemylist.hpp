
#ifndef __GNET_UPDATEENEMYLIST_HPP
#define __GNET_UPDATEENEMYLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class UpdateEnemyList : public GNET::Protocol
{
	#include "updateenemylist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
