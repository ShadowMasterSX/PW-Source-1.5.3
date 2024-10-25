
#ifndef __GNET_PLAYERKICKOUT_RE_HPP
#define __GNET_PLAYERKICKOUT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerKickout_Re : public GNET::Protocol
{
	#include "playerkickout_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
