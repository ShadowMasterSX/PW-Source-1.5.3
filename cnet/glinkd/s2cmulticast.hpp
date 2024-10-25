
#ifndef __GNET_S2CMULTICAST_HPP
#define __GNET_S2CMULTICAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "player"
#include "gamedatasend.hpp"
#include "glinkserver.hpp"

#include <algorithm>
#include <sys/time.h>
namespace GNET
{

class S2CMulticast : public GNET::Protocol
{
	#include "s2cmulticast"
	struct GetPlayerID
	{
		unsigned int operator () (const Player& player) const{
			return player.localsid;
		}
	};
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GamedataSend gds(data);
		if (! playerlist.GetVector().size() )
			return;
		GLinkServer::GetInstance()->AccumulateSend( playerlist.GetVector().begin(), playerlist.GetVector().end(),
				gds, GetPlayerID());
	}
};

};

#endif
