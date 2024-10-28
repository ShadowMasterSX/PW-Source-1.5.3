
#ifndef __GNET_FACTIONUNIQUEIDANNOUNCE_HPP
#define __GNET_FACTIONUNIQUEIDANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionUniqueIdAnnounce : public GNET::Protocol
{
	#include "factionuniqueidannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
