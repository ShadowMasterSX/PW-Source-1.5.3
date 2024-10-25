
#ifndef __GNET_DELROLEANNOUNCE_HPP
#define __GNET_DELROLEANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "accessdb.h"
namespace GNET
{

class DelRoleAnnounce : public GNET::Protocol
{
	#include "delroleannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if ( this->rolelist.size() )
			DEBUG_PRINT("delroleannounce received. size=%d\n",rolelist.size());
		GetTimeoutRole(this->rolelist);
		manager->Send(sid,this);
	}
};

};

#endif
