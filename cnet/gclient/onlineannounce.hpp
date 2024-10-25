
#ifndef __GNET_ONLINEANNOUNCE_HPP
#define __GNET_ONLINEANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "rolelist.hpp"

namespace GNET
{
class OnlineAnnounce : public GNET::Protocol
{
	#include "onlineannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkClient * cm=(GLinkClient*) manager;
		cm->SetUser(userid);
		cm->ChangeState(sid,&state_GSelectRoleClient);
		cm->SendProtocol(RoleList(userid,_SID_INVALID,_HANDLE_BEGIN));
		DEBUG_PRINT("Recv Protocol OnlineAnnounce. acc=%.*s userid=%d\n",cm->identity.size(),cm->identity.begin(),userid);	
	}
};

};

#endif
