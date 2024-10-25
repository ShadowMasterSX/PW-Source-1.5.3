
#ifndef __GNET_GETPLAYERFACTIONINFO_HPP
#define __GNET_GETPLAYERFACTIONINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "factiondb.h"

#include "getplayerfactioninfo_re.hpp"
namespace GNET
{

class GetPlayerFactionInfo : public GNET::Protocol
{
	#include "getplayerfactioninfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GetPlayerFactionInfo_Re gpf_re( roleid,localsid,GUserFaction() );
		if  ( Factiondb::GetInstance()->FindUserFaction(roleid,gpf_re.faction_info,REASON_FETCH) )
		{
			manager->Send( sid,gpf_re );
		}
	}
};

};

#endif
