
#ifndef __GNET_FACTIONLISTMEMBER_RE_HPP
#define __GNET_FACTIONLISTMEMBER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "fmemberinfo"
#include "glinkserver.hpp"
namespace GNET
{

class FactionListMember_Re : public GNET::Protocol
{
	#include "factionlistmember_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if ( GLinkServer::IsRoleOnGame( localsid ) )
			GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
