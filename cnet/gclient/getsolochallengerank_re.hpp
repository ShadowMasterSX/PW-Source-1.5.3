
#ifndef __GNET_GETSOLOCHALLENGERANK_RE_HPP
#define __GNET_GETSOLOCHALLENGERANK_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "solochallengerankdata"

namespace GNET
{

class GetSoloChallengeRank_Re : public GNET::Protocol
{
	#include "getsolochallengerank_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
