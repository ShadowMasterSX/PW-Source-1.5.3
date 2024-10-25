
#ifndef __GNET_COUNTRYBATTLEMOVE_HPP
#define __GNET_COUNTRYBATTLEMOVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattleman.h"
#include "countrybattlemove_re.hpp"

namespace GNET
{

class CountryBattleMove : public GNET::Protocol
{
	#include "countrybattlemove"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int ret = -1;
		if(dest >= 0) {
			ret = CountryBattleMan::GetInstance()->OnPlayerMove(roleid, dest);
		} else {
			ret = CountryBattleMan::GetInstance()->OnPlayerStopMove(roleid);
		}
		
		CountryBattleMove_Re re(ret, roleid, dest, localsid);
		manager->Send(sid, re);
	}
};

};

#endif
