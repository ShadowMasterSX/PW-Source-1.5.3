
#ifndef __GNET_TANKBATTLEPLAYERSCOREUPDATE_HPP
#define __GNET_TANKBATTLEPLAYERSCOREUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattleplayerscoreinfo"

namespace GNET
{

class TankBattlePlayerScoreUpdate : public GNET::Protocol
{
	#include "tankbattleplayerscoreupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
