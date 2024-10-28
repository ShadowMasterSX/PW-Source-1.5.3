
#ifndef __GNET_MNGETFACTIONINFO_RE_HPP
#define __GNET_MNGETFACTIONINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactioninfo"

namespace GNET
{

class MNGetFactionInfo_Re : public GNET::Protocol
{
	#include "mngetfactioninfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG,"file=%s\tfunc=%s\tline=%d\tretcode=%d\tlocalsid=%d\t\n", __FILE__, __FUNCTION__, __LINE__, retcode, localsid);
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp, this);
	}
};

};

#endif
