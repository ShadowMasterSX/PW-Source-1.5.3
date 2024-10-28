
#ifndef __GNET_GETSOLOCHALLENGERANK_HPP
#define __GNET_GETSOLOCHALLENGERANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetSoloChallengeRank : public GNET::Protocol
{
	#include "getsolochallengerank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
