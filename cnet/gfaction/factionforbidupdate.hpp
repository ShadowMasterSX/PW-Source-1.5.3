
#ifndef __GNET_FACTIONFORBIDUPDATE_HPP
#define __GNET_FACTIONFORBIDUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"

namespace GNET
{

class FactionForbidUpdate : public GNET::Protocol
{
	#include "factionforbidupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
