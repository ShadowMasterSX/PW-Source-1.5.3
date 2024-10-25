
#ifndef __GNET_SWITCHSERVERSUCCESS_HPP
#define __GNET_SWITCHSERVERSUCCESS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "log.h"
#include "glinkserver.hpp"
#include "playerstatussync.hpp"
#include "gproviderserver.hpp"
namespace GNET
{

class SwitchServerSuccess : public GNET::Protocol
{
	#include "switchserversuccess"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int game_id = GProviderServer::GetInstance()->FindGameServer(sid);
		if (game_id == _GAMESERVER_ID_INVALID || game_id != dst_gsid)
		{
			Log::log(LOG_ERR,"glinkd::switchserversuccess, invalid gameserver.");
			return;
		}			
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidRole(localsid,roleid))
		{
			//send playerstatusSync to gameserver
			manager->Send(sid,PlayerStatusSync(roleid,link_id,localsid,_STATUS_OFFLINE,GProviderServer::GetInstance()->FindGameServer(sid)));

			Log::log(LOG_ERR,"glinkd::switchserversuccess, invalid roleinfo(roleid=%d,localsid=%d).",roleid,localsid);
			return;
		}
		RoleData& ui=lsm->roleinfomap[roleid];
		if (!lsm->IsInSwitch(ui))
		{
			Log::log(LOG_ERR,"glinkd::switchserversuccess, Role(roleid=%d,localsid=%d) is not in switch state.",roleid,localsid);
			return; 
		}
		//1. set user's gs_id to dst_gsid
		lsm->GetSwitchUser(ui.sid,ui);
		if (ui.gs_id != dst_gsid)
		{
			Log::log(LOG_ERR,"switchserversuccess, dst_gameserver id is wrong. origin is %d, current is %d.",ui.gs_id,dst_gsid);
			return;
		}
		//2. send accumulate protocols
		lsm->SendAccumProto4Switch(ui,dst_gsid);
		//3. remove user from switchuser map
		lsm->PopSwitchUser(ui);

		//DEBUG_PRINT("glinkd::switchserversuccess: switch user(r:%d) to gameserver %d.\n",ui.roleid,ui.gs_id);
	}
};

};

#endif
