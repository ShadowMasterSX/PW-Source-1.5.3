
#ifndef __GNET_ACBRIEFINFO_HPP
#define __GNET_ACBRIEFINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACBriefInfo : public GNET::Protocol
{
	#include "acbriefinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
