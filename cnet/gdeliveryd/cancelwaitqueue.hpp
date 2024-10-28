
#ifndef __GNET_CANCELWAITQUEUE_HPP
#define __GNET_CANCELWAITQUEUE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "waitqueue.h"
#include "mapuser.h"
#include "cancelwaitqueue_re.hpp"

namespace GNET
{

class CancelWaitQueue : public GNET::Protocol
{
	#include "cancelwaitqueue"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if(!pinfo || pinfo->localsid != localsid || sid != pinfo->linksid 
				  || pinfo->status != _STATUS_ONLINE)
			return;

		if(WaitQueueManager::GetInstance()->OnCancelWait(userid))
			manager->Send(sid,CancelWaitQueue_Re(0,localsid));	
	}
};

};

#endif
