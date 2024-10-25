
#ifndef __GNET_TANKBATTLEPLAYERGETRANK_HPP
#define __GNET_TANKBATTLEPLAYERGETRANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattlemanager.h"
#include "tankbattleplayergetrank_re.hpp"

namespace GNET
{

class TankBattlePlayerGetRank : public GNET::Protocol
{
	#include "tankbattleplayergetrank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		TankBattlePlayerGetRank_Re proto;
		proto.roleid = roleid;
		proto.localsid = localsid;
		proto.retcode = TankBattleManager::GetInstance()->PlayerGetRank(roleid,proto.your_score,proto.player_scores);
	
		GDeliveryServer::GetInstance()->Send(sid, proto);
	}
};

};

#endif
