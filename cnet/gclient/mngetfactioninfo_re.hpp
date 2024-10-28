
#ifndef __GNET_MNGETFACTIONINFO_RE_HPP
#define __GNET_MNGETFACTIONINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactioninfo"

namespace GNET
{

class MNGetFactionInfo_Re : public GNET::Protocol
{
	#include "mngetfactioninfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
