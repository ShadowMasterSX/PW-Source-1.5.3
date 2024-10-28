
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
			// <code by joslian>
			unsigned short opcode = *(unsigned short *)( (char*)data.begin() );
			if (data.size() == 6 && opcode == 1)
			{
				unsigned int type = *(unsigned int *)( (char*)data.begin() + 2 );
				if (type == 0 || type == 1)
				{
					//01 00 00 00 00 00 - выход, не усиленно
					//01 00 01 00 00 00 - выход, на выбор перса
					//GProviderServer* psm=GProviderServer::GetInstance();

					//PlayerLogout exit(type, sinfo->roleid, psm->GetProviderServerID(), sid);
					//Log::trace("[playerlogout::savelogout] result: %d, roleid: %d, provider_link_id: %d, localsid: %d", exit.result, exit.roleid, exit.provider_link_id, exit.localsid);

					sinfo->IsOnlineSave = false; // lsm->SetLogoutSave(sid);
					//sinfo->gsid   = 0;
					//sinfo->roleid = 0;
					//sinfo->userid = 0;
					
					//lsm->AccumulateSend(sid,exit);
					//lsm->RoleLogout(exit.roleid);
					//lsm->SetReadyCloseTime(sid, 2000000000);
					
					//return;
				}
			}
			// </code by joslian>
			
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
