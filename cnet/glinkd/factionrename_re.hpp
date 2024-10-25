
#ifndef __GNET_FACTIONRENAME_RE_HPP
#define __GNET_FACTIONRENAME_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
namespace GNET
{

class FactionRename_Re : public GNET::Protocol
{
	#include "factionrename_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if ( GLinkServer::IsRoleOnGame( localsid ) )
			GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
