
#ifndef __GNET_DBFRIENDEXTLIST_RE_HPP
#define __GNET_DBFRIENDEXTLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfriendextinfo"

namespace GNET
{

class DBFriendExtList_Re : public GNET::Protocol
{
	#include "dbfriendextlist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
