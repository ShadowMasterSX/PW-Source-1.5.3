
#ifndef __GNET_SWITCHSERVERSUCCESS_HPP
#define __GNET_SWITCHSERVERSUCCESS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include <gproviderserver.hpp>
#include <playerstatusannounce.hpp>
#include "mapuser.h"
namespace GNET
{

class SwitchServerSuccess : public GNET::Protocol
{
	#include "switchserversuccess"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
		if(!UserContainer::GetInstance().CheckSwitch(pinfo,roleid,link_id,localsid,manager,sid))
			return;
		if (pinfo->user->switch_gsid == _GAMESERVER_ID_INVALID)
		{
			Log::log(LOG_ERR,"SwitchServerSuccess:: user(r:%d) is not in switch state.pinfo(%d),pinfo->user(%d)(linksid=%d,localsid=%d,status=%d)",pinfo->roleid,(int)pinfo,(int)pinfo->user,pinfo->user->linksid,pinfo->user->localsid,pinfo->user->status);
		   	return;
		}
		//switch user's gs_id to dst_gsid
		//todo role->gsid?
		int src_gsid = pinfo->gameid;
		pinfo->gameid = dst_gsid;
		pinfo->user->gameid = dst_gsid;
		pinfo->user->switch_gsid = _GAMESERVER_ID_INVALID;
		DEBUG_PRINT("gdelivery::switchserversuccess: switch user(r:%d) from gameserver %d to gameserver %d.",pinfo->roleid,src_gsid,pinfo->gameid);
		
		/* send playerstatusannounce to gameservers */
		PlayerStatusAnnounce psa=PlayerStatusAnnounce();
		psa.status=_STATUS_ONGAME;
		psa.playerstatus_list.add(OnlinePlayerStatus(pinfo->roleid,pinfo->gameid,pinfo->user->linkid,pinfo->localsid));
		GProviderServer::GetInstance()->BroadcastProtocol(psa);
		GFactionClient::GetInstance()->SendProtocol(psa);
	}
};

};

#endif
