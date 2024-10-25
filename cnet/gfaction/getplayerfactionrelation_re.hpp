
#ifndef __GNET_GETPLAYERFACTIONRELATION_RE_HPP
#define __GNET_GETPLAYERFACTIONRELATION_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetPlayerFactionRelation_Re : public GNET::Protocol
{
	#include "getplayerfactionrelation_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
