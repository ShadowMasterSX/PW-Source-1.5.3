
#ifndef __GNET_ANNOUNCEPROVIDERID_HPP
#define __GNET_ANNOUNCEPROVIDERID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void OnDeliveryConnected();
#include "gproviderclient.hpp"
#include "log.h"
namespace GNET
{

class AnnounceProviderID : public GNET::Protocol
{
	#include "announceproviderid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("gamed(%d)::provider server id is %d\n",GProviderClient::GetGameServerID(),id);
		printf("receive announceproviderid %d\n", id);
		if (!GProviderClient::Attach(id,(GProviderClient*)manager))
		{
			manager->Close(sid);
			Log::log(LOG_ERR,"gamed:: identical provider server id exists. connection failed. Check \".conf\" file of linkserver and delivery server and faction server.");
			return;
		}else if(id==0)
		{
			DEBUG_PRINT("gamed:: battle server register\n");
			OnDeliveryConnected();
		}
	}	
};

};

#endif
