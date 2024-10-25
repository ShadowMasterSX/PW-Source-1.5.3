
#ifndef __GNET_DISCONNECTPLAYER_HPP
#define __GNET_DISCONNECTPLAYER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
#include "maplinkserver.h"
namespace GNET
{

class DisconnectPlayer : public GNET::Protocol
{
	#include "disconnectplayer"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
		if ( pinfo )
		{
			if(pinfo->user->switch_gsid==_GAMESERVER_ID_INVALID && pinfo->gameid==gameid)
			{
				GDeliveryServer* dsm = GDeliveryServer::GetInstance();
				dsm->Send(pinfo->linksid,this);
			}
		}
		else
			LinkServer::GetInstance().BroadcastProtocol( this );
	}
};

};

#endif
