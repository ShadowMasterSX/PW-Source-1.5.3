
#ifndef __GNET_MNGETTOPLIST_RE_HPP
#define __GNET_MNGETTOPLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactionbriefinfo"

namespace GNET
{

class MNGetTopList_Re : public GNET::Protocol
{
	#include "mngettoplist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG, "file=%s\tfunc=%s\tline=%d\tlocalsid=%d\tretcode=%d\n", __FILE__, __FUNCTION__, __LINE__, localsid, retcode);
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp, this);
	}
};

};

#endif
