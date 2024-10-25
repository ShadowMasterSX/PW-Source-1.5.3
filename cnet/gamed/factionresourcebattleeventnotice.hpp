
#ifndef __GNET_FACTIONRESOURCEBATTLEEVENTNOTICE_HPP
#define __GNET_FACTIONRESOURCEBATTLEEVENTNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gfactionresourcebattlerole"

namespace GNET
{

class FactionResourceBattleEventNotice : public GNET::Protocol
{
	#include "factionresourcebattleeventnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
