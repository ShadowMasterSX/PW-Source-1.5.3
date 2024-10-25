
#ifndef __GNET_GETPLAYERBRIEFINFO_RE_HPP
#define __GNET_GETPLAYERBRIEFINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "playerbriefinfo"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class GetPlayerBriefInfo_Re : public GNET::Protocol
{
	#include "getplayerbriefinfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
