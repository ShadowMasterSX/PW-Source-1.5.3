
#ifndef __GNET_MNFACTIONCACHEGET_HPP
#define __GNET_MNFACTIONCACHEGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "cdsmnfbattleman.h"

namespace GNET
{

class MNFactionCacheGet : public GNET::Protocol
{
	#include "mnfactioncacheget"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->SendMNFBattleCache(zoneid);
	}
};

};

#endif
