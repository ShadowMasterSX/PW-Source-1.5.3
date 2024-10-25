
#ifndef __GNET_SWITCHSERVERCANCEL_HPP
#define __GNET_SWITCHSERVERCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "log.h"
#include "glinkserver.hpp"
#include "gproviderserver.hpp"
#include "playerstatussync.hpp"
#include "gproviderserver.hpp"
namespace GNET
{

class SwitchServerCancel : public GNET::Protocol
{
	#include "switchservercancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int game_id = GProviderServer::GetInstance()->FindGameServer(sid);
		if (game_id == _GAMESERVER_ID_INVALID)
		{
			Log::log(LOG_ERR,"glinkd::switchservercancel, invalid gameserver.");
			return;
		}			
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidRole(localsid,roleid))
		{
			//send playerstatusSync to gameserver
			manager->Send(sid,PlayerStatusSync(roleid,link_id,localsid,_STATUS_OFFLINE,GProviderServer::GetInstance()->FindGameServer(sid)));

			Log::log(LOG_ERR,"glinkd::switchservercancel, invalid roleinfo(roleid=%d,localsid=%d).",roleid,localsid);
			return;
		}
		
		RoleData ui;
		{
			ui=lsm->roleinfomap[roleid];
		}
		if (!lsm->IsInSwitch(ui))
		{
			Log::log(LOG_ERR,"glinkd::switchservercancel, Role(roleid=%d,localsid=%d) is not in switch state.",roleid,localsid);
			return; 
		}
		if (ui.gs_id != game_id)
		{
			Log::log(LOG_ERR,"glinkd::SwitchServerCancel:: cancel message does not come from source gameserver.\n");
			return;
		}
		//remove user from switchuser map
		lsm->PopSwitchUser(ui);

		DEBUG_PRINT("glinkd::switchservercancel: switch server canceled. user(r:%d)\n",roleid);
	}
};

};

#endif
