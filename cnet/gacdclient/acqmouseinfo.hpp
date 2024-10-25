
#ifndef __GNET_ACQMOUSEINFO_HPP
#define __GNET_ACQMOUSEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"
#include "acmouseinfo"

namespace GNET
{

class ACQMouseInfo : public GNET::Protocol
{
	#include "acqmouseinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
