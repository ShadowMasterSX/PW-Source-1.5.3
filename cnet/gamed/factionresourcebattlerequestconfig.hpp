
#ifndef __GNET_FACTIONRESOURCEBATTLEREQUESTCONFIG_HPP
#define __GNET_FACTIONRESOURCEBATTLEREQUESTCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void request_mafia_pvp_elements(unsigned int version);
namespace GNET
{

class FactionResourceBattleRequestConfig : public GNET::Protocol
{
	#include "factionresourcebattlerequestconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		request_mafia_pvp_elements(timestamp);
	}
};

};

#endif
