
#ifndef __GNET_SETCHATEMOTION_HPP
#define __GNET_SETCHATEMOTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SetChatEmotion : public GNET::Protocol
{
	#include "setchatemotion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
		if(pinfo)
		{
			LOG_TRACE("Set chat emotion to %d, roleid=%d", emotion, roleid);
			pinfo->emotion = emotion;
			if(pinfo->ingame)
				GFactionClient::GetInstance()->SendProtocol(this);
		}
		else
			LOG_TRACE("Set chat emotion, roleid=%d not found", roleid);
	}
};

};

#endif
