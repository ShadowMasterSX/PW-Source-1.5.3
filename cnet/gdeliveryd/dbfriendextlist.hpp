
#ifndef __GNET_DBFRIENDEXTLIST_HPP
#define __GNET_DBFRIENDEXTLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class DBFriendExtList : public GNET::Protocol
{
	#include "dbfriendextlist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
