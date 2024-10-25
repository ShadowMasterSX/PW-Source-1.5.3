
#ifndef __GNET_GETSELLLIST_RE_HPP
#define __GNET_GETSELLLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"

namespace GNET
{

class GetSellList_Re : public GNET::Protocol
{
	#include "getselllist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
