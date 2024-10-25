
#ifndef __GNET_ACQUSERONLINE_HPP
#define __GNET_ACQUSERONLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"
#include "intdata"

namespace GNET
{

class ACQUserOnline : public GNET::Protocol
{
	#include "acquseronline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
