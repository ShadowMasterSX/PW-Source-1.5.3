
#ifndef __GNET_COUNTRYBATTLEGETPLAYERLOCATION_HPP
#define __GNET_COUNTRYBATTLEGETPLAYERLOCATION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattlesyncplayerlocation.hpp"

namespace GNET
{

class CountryBattleGetPlayerLocation : public GNET::Protocol
{
	#include "countrybattlegetplayerlocation"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int domain_id = CountryBattleMan::OnPlayerGetDomainId(roleid);

		CountryBattleSyncPlayerLocation proto(roleid, domain_id, CountryBattleMan::REASON_SYNC_CLI_REQUEST, localsid);
		manager->Send(sid, proto);
	}
};

};

#endif
