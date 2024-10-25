
#ifndef __GNET_INCREASEFORCEACTIVITY_HPP
#define __GNET_INCREASEFORCEACTIVITY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "forcemanager.h"

namespace GNET
{

class IncreaseForceActivity : public GNET::Protocol
{
	#include "increaseforceactivity"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		ForceManager::GetInstance()->IncreaseActivity(force_id, activity);
	}
};

};

#endif
