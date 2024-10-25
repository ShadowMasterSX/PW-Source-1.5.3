
#ifndef __GNET_TANKBATTLEPLAYERGETRANK_HPP
#define __GNET_TANKBATTLEPLAYERGETRANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryclient.hpp"

namespace GNET
{

class TankBattlePlayerGetRank : public GNET::Protocol
{
	#include "tankbattleplayergetrank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
			return;
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
