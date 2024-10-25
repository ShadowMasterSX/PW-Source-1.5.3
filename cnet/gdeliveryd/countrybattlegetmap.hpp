
#ifndef __GNET_COUNTRYBATTLEGETMAP_HPP
#define __GNET_COUNTRYBATTLEGETMAP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattleman.h"

namespace GNET
{

class CountryBattleGetMap : public GNET::Protocol
{
	#include "countrybattlegetmap"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		CountryBattleMan::GetInstance()->SendMap(roleid, sid, localsid);
	}
};

};

#endif
