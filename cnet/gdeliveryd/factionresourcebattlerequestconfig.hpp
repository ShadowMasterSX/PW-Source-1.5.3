
#ifndef __GNET_FACTIONRESOURCEBATTLEREQUESTCONFIG_HPP
#define __GNET_FACTIONRESOURCEBATTLEREQUESTCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionResourceBattleRequestConfig : public GNET::Protocol
{
	#include "factionresourcebattlerequestconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
