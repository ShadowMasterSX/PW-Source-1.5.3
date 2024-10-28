
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
		Log::log(LOG_DEBUG,"file=%s\tfunc=%s\tline=%d\tlocalsid=%d\t\n", __FILE__, __FUNCTION__, __LINE__, localsid);
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp, this);
	}
};

};

#endif
