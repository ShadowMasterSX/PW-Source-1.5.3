
#ifndef __GNET_PLAYERSTATUSSYNC_HPP
#define __GNET_PLAYERSTATUSSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_not_online(int userid, int link_id, int sid);

namespace GNET
{

class PlayerStatusSync : public GNET::Protocol
{
	#include "playerstatussync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_not_online(roleid,provider_link_id,localsid);
	}
};

};

#endif
