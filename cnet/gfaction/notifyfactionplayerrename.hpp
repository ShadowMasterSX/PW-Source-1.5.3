
#ifndef __GNET_NOTIFYFACTIONPLAYERRENAME_HPP
#define __GNET_NOTIFYFACTIONPLAYERRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class NotifyFactionPlayerRename : public GNET::Protocol
{
	#include "notifyfactionplayerrename"


	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("Factiondb::NotifyFactionPlayerRename rid=%d\n", roleid);
		GFactionServer::Player player;
		GFactionServer* gfs = GFactionServer::GetInstance();
		if (!(gfs->GetPlayer(roleid,player)))
			return;

		//先修改帮派服务器内存里的玩家名字
		gfs->OnPlayerRename(roleid, newname);
		
		if (0 != player.faction_id)
		{
			//再修改catch里的玩家名字并通知所有帮派成员
			Factiondb::GetInstance()->OnPlayerRename(player.faction_id, roleid, newname) ;
		}
	}
};

};

#endif
