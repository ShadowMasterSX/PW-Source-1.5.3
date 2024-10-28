
#ifndef __GNET_MNGETFACTIONBRIEFINFO_HPP
#define __GNET_MNGETFACTIONBRIEFINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetFactionBriefInfo : public GNET::Protocol
{
	#include "mngetfactionbriefinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDC_MNFactionBattleMan::GetInstance()->SendMNFactionBriefInfo(fid, zoneid, roleid);
	}
};

};

#endif
