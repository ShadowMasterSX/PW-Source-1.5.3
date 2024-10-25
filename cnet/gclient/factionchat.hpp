
#ifndef __GNET_FACTIONCHAT_HPP
#define __GNET_FACTIONCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FactionChat : public GNET::Protocol
{
	#include "factionchat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("player %3d said: %.*s.\n",src_roleid,msg.size(),(char*) msg.begin());
	}
};

};

#endif
