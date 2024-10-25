
#ifndef __GNET_GETSAVEDMSG_RE_HPP
#define __GNET_GETSAVEDMSG_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "message"

namespace GNET
{

class GetSavedMsg_Re : public GNET::Protocol
{
	#include "getsavedmsg_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(localsid,roleid)) 
			return;
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp,this);	
	}
};

};

#endif
