
#ifndef __GNET_MNFETCHTOPLIST_RE_HPP
#define __GNET_MNFETCHTOPLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactionbriefinfo"

namespace GNET
{

class MNFetchTopList_Re : public GNET::Protocol
{
	#include "mnfetchtoplist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDC_MNToplistMan::GetInstance()->OnGetCDSToplist(toplist);
	}
};

};

#endif
