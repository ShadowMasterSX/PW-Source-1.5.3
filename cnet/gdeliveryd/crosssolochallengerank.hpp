
#ifndef __GNET_CROSSSOLOCHALLENGERANK_HPP
#define __GNET_CROSSSOLOCHALLENGERANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "solochallengerankdataext"
#include "crosssystem.h"

namespace GNET
{

class CrossSoloChallengeRank : public GNET::Protocol
{
	#include "crosssolochallengerank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(GDeliveryServer::GetInstance()->IsCentralDS())
			CrossSoloRankServer::GetInstance()->OnRecv(*this);
		else
			CrossSoloRankClient::GetInstance()->OnRecv(*this);
	}
};

};

#endif
