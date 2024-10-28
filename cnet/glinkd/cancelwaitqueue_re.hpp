
#ifndef __GNET_CANCELWAITQUEUE_RE_HPP
#define __GNET_CANCELWAITQUEUE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"


namespace GNET
{

class CancelWaitQueue_Re : public GNET::Protocol
{
	#include "cancelwaitqueue_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(result == 0)
			GLinkServer::GetInstance()->ChangeState(localsid,&state_GSelectRoleServer);

		int tmpsid = localsid;
		localsid = 0;
		GLinkServer::GetInstance()->Send(tmpsid,this);
	}
};

};

#endif
