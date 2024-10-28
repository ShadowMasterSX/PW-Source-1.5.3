
#ifndef __GNET_MNDOMAINBATTLELEAVENOTICE_HPP
#define __GNET_MNDOMAINBATTLELEAVENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleLeaveNotice : public GNET::Protocol
{
	#include "mndomainbattleleavenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
