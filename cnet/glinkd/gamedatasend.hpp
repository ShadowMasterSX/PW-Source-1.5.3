
#ifndef __GNET_GAMEDATASEND_HPP
#define __GNET_GAMEDATASEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gproviderserver.hpp"
#include "c2sgamedatasend.hpp"
namespace GNET
{
class GamedataSend : public GNET::Protocol
{
	#include "gamedatasend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		lsm->SetAliveTime(sid, _CLIENT_TTL);

		SessionInfo * sinfo = lsm->GetSessionInfo(sid);
		if (sinfo && sinfo->gsid) 
		{       
			sinfo->protostat.gamedatasend++;

			if (lsm->IsInSwitch(sid, sinfo->roleid))
			{
				lsm->AccumProto4Switch(sid, C2SGamedataSend(sinfo->roleid,sid,data));
			}
			else	
			{
				GProviderServer::GetInstance()->DispatchProtocol(sinfo->gsid,C2SGamedataSend(sinfo->roleid,sid,data));
			}
		}
	}
};

};

#endif
