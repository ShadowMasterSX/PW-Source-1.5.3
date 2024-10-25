
#ifndef __GNET_SENDREFCASHUSED_HPP
#define __GNET_SENDREFCASHUSED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SendRefCashUsed : public GNET::Protocol
{
	#include "sendrefcashused"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
