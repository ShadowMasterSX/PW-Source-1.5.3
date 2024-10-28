
#ifndef __GNET_MNDOMAININFOGSNOTICE_HPP
#define __GNET_MNDOMAININFOGSNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mndomaininfo"

namespace GNET
{

class MNDomainInfoGSNotice : public GNET::Protocol
{
	#include "mndomaininfogsnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		// TODO
	}
};

};

#endif
