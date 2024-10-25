
#ifndef __GNET_PLAYERBASEINFO_RE_HPP
#define __GNET_PLAYERBASEINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleforbid"
#include "grolebase"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class PlayerBaseInfo_Re : public GNET::Protocol
{
	#include "playerbaseinfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);

	}
};

};

#endif
