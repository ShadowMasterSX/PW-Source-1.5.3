
#ifndef __GNET_BATTLEENTERNOTICE_HPP
#define __GNET_BATTLEENTERNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_enter_battleground(int role_id, int server_id,int world_tag, int battle_id);
namespace GNET
{

class BattleEnterNotice : public GNET::Protocol
{
	#include "battleenternotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_enter_battleground(roleid,server_id,world_tag,battle_id);
	}
};

};

#endif
