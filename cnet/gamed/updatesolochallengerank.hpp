
#ifndef __GNET_UPDATESOLOCHALLENGERANK_HPP
#define __GNET_UPDATESOLOCHALLENGERANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class UpdateSoloChallengeRank : public GNET::Protocol
{
	#include "updatesolochallengerank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
