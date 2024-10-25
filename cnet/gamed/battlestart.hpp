
#ifndef __GNET_BATTLESTART_HPP
#define __GNET_BATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void battleground_start(int battle_id, int attacker, int defender,int end_time, int type, int map_type);
namespace GNET
{

class BattleStart : public GNET::Protocol
{
	#include "battlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		battleground_start(battle_id,attacker,defender,end_time,type,map_type);
	}
};

};

#endif
