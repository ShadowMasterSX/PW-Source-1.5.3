
#ifndef __GNET_QUERYGAMESERVERATTR_RE_HPP
#define __GNET_QUERYGAMESERVERATTR_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gameattr"
namespace GNET
{

class QueryGameServerAttr_Re : public GNET::Protocol
{
	#include "querygameserverattr_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
