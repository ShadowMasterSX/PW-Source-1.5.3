
#ifndef __GNET_DISABLEAUTOLOCK_HPP
#define __GNET_DISABLEAUTOLOCK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class DisableAutolock : public GNET::Protocol
{
	#include "disableautolock"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		UserInfo* userinfo=UserContainer::GetInstance().FindUser(userid);
		if (!userinfo) 
			return; 
		Log::formatlog("gamemaster","disableautolock:userid=%d:sid=%d", userid, sid);
		userinfo->autolock = PairSet();
	}
};

};

#endif
