
#ifndef __GNET_GETFRIENDS_RE_HPP
#define __GNET_GETFRIENDS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ggroupinfo"
#include "gfriendinfo"

namespace GNET
{

class GetFriends_Re : public GNET::Protocol
{
	#include "getfriends_re"

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
