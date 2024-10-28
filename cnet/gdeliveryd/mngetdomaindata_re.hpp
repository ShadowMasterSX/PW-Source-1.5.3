
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
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	}
};

};

#endif
