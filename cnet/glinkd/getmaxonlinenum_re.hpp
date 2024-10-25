
#ifndef __GNET_GETMAXONLINENUM_RE_HPP
#define __GNET_GETMAXONLINENUM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GetMaxOnlineNum_Re : public GNET::Protocol
{
	#include "getmaxonlinenum_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
