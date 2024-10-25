
#ifndef __GNET_FACTIONRESOURCEBATTLESTATUSNOTICE_HPP
#define __GNET_FACTIONRESOURCEBATTLESTATUSNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionResourceBattleStatusNotice : public GNET::Protocol
{
	#include "factionresourcebattlestatusnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
