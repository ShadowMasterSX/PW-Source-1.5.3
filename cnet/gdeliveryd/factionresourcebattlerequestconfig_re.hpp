
#ifndef __GNET_FACTIONRESOURCEBATTLEREQUESTCONFIG_RE_HPP
#define __GNET_FACTIONRESOURCEBATTLEREQUESTCONFIG_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionresourcebattleconfig"
#include "factionresourcebattleman.h"

namespace GNET
{

class FactionResourceBattleRequestConfig_Re : public GNET::Protocol
{
	#include "factionresourcebattlerequestconfig_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	    FactionResourceBattleMan::GetInstance()->SetConfigData(config_list, controller_list);	
	}
};

};

#endif
