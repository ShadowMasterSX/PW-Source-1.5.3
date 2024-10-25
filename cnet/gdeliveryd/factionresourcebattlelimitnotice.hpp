
#ifndef __GNET_FACTIONRESOURCEBATTLELIMITNOTICE_HPP
#define __GNET_FACTIONRESOURCEBATTLELIMITNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionresourcebattlelimit"

namespace GNET
{

class FactionResourceBattleLimitNotice : public GNET::Protocol
{
	#include "factionresourcebattlelimitnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
