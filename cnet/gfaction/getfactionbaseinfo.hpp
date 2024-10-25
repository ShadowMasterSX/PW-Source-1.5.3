
#ifndef __GNET_GETFACTIONBASEINFO_HPP
#define __GNET_GETFACTIONBASEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GetFactionBaseInfo : public GNET::Protocol
{
	#include "getfactionbaseinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("GetFactionBaseInfo, roleid=%d, factionlist.size=%d", roleid, factionlist.size());

		FactionBriefInfo info;
		GetFactionBaseInfo_Re re;
		re.roleid = roleid;
		re.localsid = localsid;
		GFactionServer::Player player;
		GFactionServer* gfs = GFactionServer::GetInstance();
		if (!(gfs->GetPlayer(roleid,player)))
			return;
		for(IntVector::iterator it=factionlist.begin();it!=factionlist.end();it++)
		{
			info.fid = *it;
			if(Factiondb::GetInstance()->FindFactionInCache(info))
			{
				re.faction_info.fid = info.fid; 
				re.faction_info.name.swap(info.name); 
				re.faction_info.level = info.level; 
				re.faction_info.mem_num = info.mem_num; 
				if(!gfs->DispatchProtocol(player.linkid, re))
					return;
			}
		}
	}
};

};

#endif
