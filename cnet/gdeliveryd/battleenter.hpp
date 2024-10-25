
#ifndef __GNET_BATTLEENTER_HPP
#define __GNET_BATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class BattleEnter : public GNET::Protocol
{
	#include "battleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		BattleManager* pmanager =  BattleManager::GetInstance();
		int world  =  pmanager->GetMapType(battle_id);
		int server = pmanager->FindServer(world);

		LOG_TRACE( "BattleEnter: roleid=%d battle_id=%d world=%d server=%d.\n", roleid, battle_id,world,server);
		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			BattleEnterNotice notice(0, roleid, battle_id, server, world);

			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
			if ( NULL!=pinfo )
				GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, notice);
		}
	}
};

};

#endif
