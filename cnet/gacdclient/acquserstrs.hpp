
#ifndef __GNET_ACQUSERSTRS_HPP
#define __GNET_ACQUSERSTRS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"

namespace GNET
{

class ACQUserStrs : public GNET::Protocol
{
	#include "acquserstrs"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
