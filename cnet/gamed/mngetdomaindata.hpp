
#ifndef __GNET_MNGETDOMAINDATA_HPP
#define __GNET_MNGETDOMAINDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetDomainData : public GNET::Protocol
{
	#include "mngetdomaindata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
