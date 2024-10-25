
#ifndef __GNET_AUTOTEAMCONFIGREGISTER_HPP
#define __GNET_AUTOTEAMCONFIGREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "autoteamconfigdata"
#include "autoteamman.h"

namespace GNET
{

class AutoTeamConfigRegister : public GNET::Protocol
{
	#include "autoteamconfigregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::formatlog("autoteam", "config_register=data_cnt=%d:sid=%d", config_data.size(), sid);
		AutoTeamMan::GetInstance()->RegisterGoalConfig(config_data);	
	}
};

};

#endif
