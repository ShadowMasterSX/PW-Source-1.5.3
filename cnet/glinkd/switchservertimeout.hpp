
#ifndef __GNET_SWITCHSERVERTIMEOUT_HPP
#define __GNET_SWITCHSERVERTIMEOUT_HPP

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

class SwitchServerTimeout : public GNET::Protocol
{
	#include "switchservertimeout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int game_id = GProviderServer::GetInstance()->FindGameServer(sid);
		if (game_id == _GAMESERVER_ID_INVALID)
		{
			Log::log(LOG_ERR,"glinkd::switchservertimeout, invalid gameserver.");
			return;
		}			
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (!lsm->ValidRole(localsid,roleid))
		{
			//send playerstatusSync to gameserver
			manager->Send(sid,PlayerStatusSync(roleid,link_id,localsid,_STATUS_OFFLINE,GProviderServer::GetInstance()->FindGameServer(sid)));
			Log::log(LOG_ERR,"glinkd::switchservertimeout, invalid roleinfo(roleid=%d,localsid=%d).",roleid,localsid);
			return;
		}
		RoleData& ui=lsm->roleinfomap[roleid];
		if (!lsm->IsInSwitch(ui))
		{
			Log::log(LOG_ERR,"switchservertimeout, Role(roleid=%d,localsid=%d) is not in switch state.",roleid,localsid);
			return; 
		}
		//remove user from switchuser map
		lsm->PopSwitchUser(ui);
		//disconnect the client
		//lsm->SessionError(ui.sid,ERR_TIMEOUT,"Server timeout, when switching server.");
		lsm->Close(ui.sid);
		lsm->ActiveCloseSession(ui.sid);
		
		DEBUG_PRINT("glinkd:: switchservertimeout: game server timeout. Disconnect the player sid=%d, roleid=%d.\n",ui.sid,roleid);
	}
};

};

#endif
