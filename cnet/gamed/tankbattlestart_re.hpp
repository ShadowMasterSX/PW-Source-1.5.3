
#ifndef __GNET_TANKBATTLESTART_RE_HPP
#define __GNET_TANKBATTLESTART_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattleStart_Re : public GNET::Protocol
{
	#include "tankbattlestart_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
