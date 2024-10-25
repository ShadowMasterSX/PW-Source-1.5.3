
#ifndef __GNET_FACTIONINFOUPDATE_HPP
#define __GNET_FACTIONINFOUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "factioninfobean"
#include "gametalkmanager.h"

namespace GNET
{

class FactionInfoUpdate : public GNET::Protocol
{
	#include "factioninfoupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GameTalkManager::GetInstance()->NotifyUpdate(*this);
	}
};

};

#endif
