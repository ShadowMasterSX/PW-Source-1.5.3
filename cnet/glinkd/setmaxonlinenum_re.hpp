
#ifndef __GNET_SETMAXONLINENUM_RE_HPP
#define __GNET_SETMAXONLINENUM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SetMaxOnlineNum_Re : public GNET::Protocol
{
	#include "setmaxonlinenum_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
