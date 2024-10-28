
#ifndef __GNET_CROSSSOLOCHALLENGERANK_RE_HPP
#define __GNET_CROSSSOLOCHALLENGERANK_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "crosssystem.h"

namespace GNET
{

class CrossSoloChallengeRank_Re : public GNET::Protocol
{
	#include "crosssolochallengerank_re"

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
