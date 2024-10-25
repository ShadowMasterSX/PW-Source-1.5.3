
#ifndef __GNET_ANNOUNCECHALLENGEALGO_HPP
#define __GNET_ANNOUNCECHALLENGEALGO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class AnnounceChallengeAlgo : public GNET::Protocol
{
	#include "announcechallengealgo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
