
#ifndef __GNET_MNDOMAINSENDBONUS_HPP
#define __GNET_MNDOMAINSENDBONUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainSendBonus : public GNET::Protocol
{
	#include "mndomainsendbonus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
