
#ifndef __GNET_ACQTHREADTIMES_HPP
#define __GNET_ACQTHREADTIMES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"
#include "acthreadtime"
#include "acthreadtime"

namespace GNET
{

class ACQThreadTimes : public GNET::Protocol
{
	#include "acqthreadtimes"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
