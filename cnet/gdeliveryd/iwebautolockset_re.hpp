
#ifndef __GNET_IWEBAUTOLOCKSET_RE_HPP
#define __GNET_IWEBAUTOLOCKSET_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class IWebAutolockSet_Re : public GNET::Protocol
{
	#include "iwebautolockset_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
