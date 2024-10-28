
#ifndef __GNET_MNDOMAINBATTLEENTERSUCCESSNOTICE_HPP
#define __GNET_MNDOMAINBATTLEENTERSUCCESSNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleEnterSuccessNotice : public GNET::Protocol
{
	#include "mndomainbattleentersuccessnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
