
#ifndef __GNET_BATTLEMAPNOTICE_HPP
#define __GNET_BATTLEMAPNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gcity"

namespace GNET
{

class BattleMapNotice : public GNET::Protocol
{
	#include "battlemapnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
