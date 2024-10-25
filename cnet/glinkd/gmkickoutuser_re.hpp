
#ifndef __GNET_GMKICKOUTUSER_RE_HPP
#define __GNET_GMKICKOUTUSER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"
namespace GNET
{

class GMKickoutUser_Re : public GNET::Protocol
{
	#include "gmkickoutuser_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
