
#ifndef __GNET_AUMAILSENDED_HPP
#define __GNET_AUMAILSENDED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_get_aumail_task(int roleid, int level, char ext_reward);
namespace GNET
{

class AUMailSended : public GNET::Protocol
{
	#include "aumailsended"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		player_get_aumail_task(roleid,level,ext_reward);
	}
};

};

#endif
