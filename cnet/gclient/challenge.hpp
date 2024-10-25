
#ifndef __GNET_CHALLENGE_HPP
#define __GNET_CHALLENGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "response.hpp"

#include "glinkclient.hpp"
namespace GNET
{

class Challenge : public GNET::Protocol
{
	#include "challenge"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkClient * cm=(GLinkClient*) manager;
		Response response;
		response.Setup(cm->identity,cm->password,nonce);
		cm->SendProtocol(response);
		cm->password.swap(response.response);	
		//DEBUG_PRINT("Recv Protocol Challenge.\n");
		return;
	}
};

};

#endif
