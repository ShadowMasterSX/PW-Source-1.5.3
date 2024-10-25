
#ifndef __GNET_FACTIONRESOURCEBATTLESERVERREGISTER_HPP
#define __GNET_FACTIONRESOURCEBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionResourceBattleServerRegister : public GNET::Protocol
{
	#include "factionresourcebattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
