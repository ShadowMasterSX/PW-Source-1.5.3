
#ifndef __GNET_MNFACTIONCACHEGET_RE_HPP
#define __GNET_MNFACTIONCACHEGET_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "cdcmnfbattleman.h"
namespace GNET
{

class MNFactionCacheGet_Re : public GNET::Protocol
{
	#include "mnfactioncacheget_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDC_MNFactionBattleMan::GetInstance()->OnGetCDSBattleCache(sn, apply_begin_time, apply_end_time, cross_begin_time, battle_begin_time, battle_end_time, domaininfo_list, factioninfo_list);
	}
};

};

#endif
