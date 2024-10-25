
#ifndef __GNET_FACTIONFORTRESSENTERNOTICE_HPP
#define __GNET_FACTIONFORTRESSENTERNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


void player_enter_faction_fortress(int role_id, int dst_world_tag, int dst_factionid);
namespace GNET
{

class FactionFortressEnterNotice : public GNET::Protocol
{
	#include "factionfortressenternotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_enter_faction_fortress(roleid,dst_world_tag,dst_factionid);
	}
};

};

#endif
