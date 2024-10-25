
#ifndef __GNET_GMLISTONLINEUSER_RE_HPP
#define __GNET_GMLISTONLINEUSER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmplayerinfo"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"
namespace GNET
{

class GMListOnlineUser_Re : public GNET::Protocol
{
	#include "gmlistonlineuser_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("glinkd::send gmlistonlineuser_re to client.\n");
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
