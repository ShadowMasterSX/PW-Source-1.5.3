
#ifndef __GNET_DELFRIEND_RE_HPP
#define __GNET_DELFRIEND_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DelFriend_Re : public GNET::Protocol
{
	#include "delfriend_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(localsid,roleid)) 
			return;
		unsigned int tmp = localsid;
		localsid = 0;
		GLinkServer::GetInstance()->Send(tmp,this);	
	}
};

};

#endif
