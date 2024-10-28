
#ifndef __GNET_MNGETDOMAINDATA_RE_HPP
#define __GNET_MNGETDOMAINDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mndomaindata"
#include "mnfactionapplydata"

namespace GNET
{

class MNGetDomainData_Re : public GNET::Protocol
{
	#include "mngetdomaindata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
