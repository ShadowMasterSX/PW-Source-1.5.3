
#ifndef __GNET_MNFACTIONPROCLAIM_HPP
#define __GNET_MNFACTIONPROCLAIM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactionapplyinfo"

namespace GNET
{

class MNFactionProclaim : public GNET::Protocol
{
	#include "mnfactionproclaim"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%dzoneid=%d\tapplyinfo_list.size=%d\tfaction.size()=%d\tlvl3.size()=%d\n", __FILE__, __FUNCTION__, __LINE__, zoneid, applyinfo_list.size(),factioninfo_list.size(), lvl3_list.size());
		CDS_MNFactionBattleMan::GetInstance()->OnGetMNFactionProclaim(zoneid, applyinfo_list, factioninfo_list, lvl3_list);
	}
};

};

#endif
