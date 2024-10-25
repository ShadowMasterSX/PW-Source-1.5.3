
#ifndef __GNET_ACQSTROWNER_HPP
#define __GNET_ACQSTROWNER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"
#include "intdata"

namespace GNET
{

class ACQStrOwner : public GNET::Protocol
{
	#include "acqstrowner"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
