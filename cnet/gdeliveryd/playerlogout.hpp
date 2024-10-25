
#ifndef __GNET_PLAYERLOGOUT_HPP
#define __GNET_PLAYERLOGOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "log.h"
#include "mapuser.h"
namespace GNET
{

class PlayerLogout : public GNET::Protocol
{
	#include "playerlogout"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(result!=_PLAYER_LOGOUT_HALF && result!=_PLAYER_LOGOUT_FULL)
		{
			Log::log(LOG_ERR,"gdelivery::playerlogout, retcode is invalid.\n");
			return;
		}

		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if(dsm->IsCentralDS() && result == _PLAYER_LOGOUT_HALF) {
			result = _PLAYER_LOGOUT_FULL;
			Log::log(LOG_ERR, "roleid %d try to half logout on Central Delivery Server, convert to full logout!", roleid);
		}

		UserContainer::GetInstance().OnPlayerLogout(*this);
	}
};

};

#endif
