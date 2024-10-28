
#ifndef __GNET_MNDOMAINSENDBONUS_RE_HPP
#define __GNET_MNDOMAINSENDBONUS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainSendBonus_Re : public GNET::Protocol
{
	#include "mndomainsendbonus_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
