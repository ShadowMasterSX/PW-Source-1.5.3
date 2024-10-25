
#ifndef __GNET_PLAYERHEARTBEAT_HPP
#define __GNET_PLAYERHEARTBEAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "playerstatussync.hpp"

#include "gproviderserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class PlayerHeartBeat : public GNET::Protocol
{
	#include "playerheartbeat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidRole(localsid,roleid))
		{
			//send playeroffline to gameserver
			GDeliveryClient::GetInstance()->SendProtocol(PlayerStatusSync(roleid,link_id,localsid,_STATUS_OFFLINE,
				GProviderServer::GetInstance()->FindGameServer(sid)));
			Log::log(LOG_ERR,"glinkd::PlayerHearBeat, invalid roleinfo(roleid=%d,localsid=%d).",roleid,localsid);
			return;
		}

	}
};

};

#endif
