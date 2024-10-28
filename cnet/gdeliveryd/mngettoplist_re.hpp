
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
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	}
};

};

#endif
