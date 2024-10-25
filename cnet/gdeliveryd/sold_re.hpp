
#ifndef __GNET_SOLD_RE_HPP
#define __GNET_SOLD_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class Sold_Re : public GNET::Protocol
{
	#include "sold_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
