
#ifndef __GNET_TANKBATTLEPLAYERGETRANK_RE_HPP
#define __GNET_TANKBATTLEPLAYERGETRANK_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattleplayerscoreinfo"

#include "glinkserver.hpp"

namespace GNET
{

class TankBattlePlayerGetRank_Re : public GNET::Protocol
{
	#include "tankbattleplayergetrank_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer * lsm = GLinkServer::GetInstance();
		int lsid = localsid;
		localsid = 0;
		lsm->Send(lsid, this);
	}
};

};

#endif
