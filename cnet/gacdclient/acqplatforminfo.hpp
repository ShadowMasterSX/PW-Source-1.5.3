
#ifndef __GNET_ACQPLATFORMINFO_HPP
#define __GNET_ACQPLATFORMINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"
#include "acplatforminfo"
#include "accpuinfo"
#include "acmeminfo"
#include "acadapterinfo"

namespace GNET
{

class ACQPlatformInfo : public GNET::Protocol
{
	#include "acqplatforminfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
