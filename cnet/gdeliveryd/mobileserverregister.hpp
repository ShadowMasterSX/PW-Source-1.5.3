
#ifndef __GNET_MOBILESERVERREGISTER_HPP
#define __GNET_MOBILESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"


namespace GNET
{

class MobileServerRegister : public GNET::Protocol
{
	#include "mobileserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GProviderServer::GetInstance()->InsertPhoneGS(server_id))
			Log::log(LOG_ERR,"MobileServerRegister gameid=%d already exist!", server_id);
	}
};

};

#endif
