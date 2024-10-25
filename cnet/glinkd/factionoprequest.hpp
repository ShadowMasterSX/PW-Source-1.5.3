
#ifndef __GNET_FACTIONOPREQUEST_HPP
#define __GNET_FACTIONOPREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gfactionclient.hpp"
#include "gproviderserver.hpp"
#include "factionoprequest_re.hpp"
#include "../gfaction/operations/ids.hxx"
namespace GNET
{

class FactionOPRequest : public GNET::Protocol
{
	#include "factionoprequest"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		switch(optype)
		{
			//这些操作必须通过gamed转发
			case _O_FACTION_CREATE:
			case _O_FACTION_UPGRADE:
			case _O_FACTION_ACCELERATEEXPELSCHEDULE:
			case _O_FACTION_TESTSYNC:    
				return;
			default:
				break;
		}
		if ( !GFactionClient::GetInstance()->SendProtocol(this) )	
			GLinkServer::GetInstance()->Send( sid,FactionOPRequest_Re(ERR_FC_NETWORKERR,optype,roleid,-1,_SID_INVALID) );
	}
};

};

#endif
