
#ifndef __GNET_FACTIONLISTAPPLICANT_RE_HPP
#define __GNET_FACTIONLISTAPPLICANT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
namespace GNET
{

class FactionListApplicant_Re : public GNET::Protocol
{
	#include "factionlistapplicant_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if ( GLinkServer::IsRoleOnGame( localsid ) )
			GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
