
#ifndef __GNET_ACRELOADCONFIG_HPP
#define __GNET_ACRELOADCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acreloadconfigre.hpp"
#include "configmanager.hpp"
#include "gaccontrolserver.hpp"

namespace GNET
{

class ACReloadConfig : public GNET::Protocol
{
	#include "acreloadconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		ConfigManager::GetInstance()->Reload((const char*)config.begin(), config.size());
		((GACControlServer*)manager)->SendProtocol(ACReloadConfigRe(), sid);
	}
};

};

#endif
