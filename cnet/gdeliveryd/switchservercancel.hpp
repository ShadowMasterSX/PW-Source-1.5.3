
#ifndef __GNET_SWITCHSERVERCANCEL_HPP
#define __GNET_SWITCHSERVERCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "mapuser.h"
namespace GNET
{

class SwitchServerCancel : public GNET::Protocol
{
	#include "switchservercancel"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager == GProviderServer::GetInstance())
		{
			int game_id=GProviderServer::GetInstance()->FindGameServer(sid);
			if (game_id == _GAMESERVER_ID_INVALID) 
			{
				Log::log(LOG_ERR,"gdelivery::switchservercancel: invalid gameserver.");
				return;
			}	
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			if(!UserContainer::GetInstance().CheckSwitch(pinfo,roleid,link_id,localsid,manager,sid))
				return;
			if (pinfo->gameid != game_id)
			{
				Log::log(LOG_ERR,"gdelivery::SwitchServerCancel:: cancel message does not come from source gameserver.\n");
				return;
			}
			//set switch_gs_id to invalid id
			pinfo->user->switch_gsid = _GAMESERVER_ID_INVALID;
			DEBUG_PRINT("gdelivery::SwitchServerCancel:  cancel successfully. user(r:%d, gs_id:%d).pinfo(%d),pinfo->user(%d)\n",pinfo->roleid,pinfo->gameid,(int)pinfo,(int)pinfo->user);
		}
		else if(manager == GDeliveryServer::GetInstance())
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			if(NULL == pinfo)
			{
				Log::log(LOG_ERR,"gdelivery::SwitchServerCancel:: user not found, roleid=%d,linkid=%d,localsid=%d",roleid,link_id,localsid);
				return;
			}
			if(pinfo->user->linkid != link_id || pinfo->localsid != localsid)
			{
				Log::log(LOG_ERR,"gdelivery::SwitchServerCancel:: status error, roleid=%d,linkid=%d,localsid=%d",roleid,link_id,localsid);
				return;
			}
			//set switch_gs_id to invalid id
			pinfo->user->switch_gsid = _GAMESERVER_ID_INVALID;
			DEBUG_PRINT("gdelivery::SwitchServerCancel2:  cancel successfully. user(r:%d, gs_id:%d).pinfo(%d),pinfo->user(%d)\n",pinfo->roleid,pinfo->gameid,(int)pinfo,(int)pinfo->user);
		}
	}
};

};

#endif
