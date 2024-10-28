
#ifndef __GNET_CANCELWAITQUEUE_HPP
#define __GNET_CANCELWAITQUEUE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"


namespace GNET
{

class CancelWaitQueue : public GNET::Protocol
{
	#include "cancelwaitqueue"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidUser(sid,userid)) {
			DEBUG_PRINT("LinkServer::CancelWaitQueue Err:: Session %d,(userid=%d)\n",sid,userid);
			return;
		}

		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
