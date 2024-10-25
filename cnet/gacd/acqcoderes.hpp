
#ifndef __GNET_ACQCODERES_HPP
#define __GNET_ACQCODERES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acq"
#include "accoderes"
#include "acusercoderes"

namespace GNET
{

class ACQCodeRes : public GNET::Protocol
{
	#include "acqcoderes"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
