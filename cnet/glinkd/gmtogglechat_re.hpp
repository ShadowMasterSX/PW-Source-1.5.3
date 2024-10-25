
#ifndef __GNET_GMTOGGLECHAT_RE_HPP
#define __GNET_GMTOGGLECHAT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"

namespace GNET
{

class GMToggleChat_Re : public GNET::Protocol
{
	#include "gmtogglechat_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("glinkd::send gmtogglechat_re to client.\n");
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
