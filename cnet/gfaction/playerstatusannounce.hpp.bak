
#ifndef __GNET_PLAYERSTATUSANNOUNCE_HPP
#define __GNET_PLAYERSTATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "onlineplayerstatus"
#include "gfactionserver.hpp"
#include "gproviderserver.hpp"
#include "playerfactioninfo_re.hpp"

#include "factiondb.h"
namespace GNET
{

class PlayerStatusAnnounce : public GNET::Protocol
{
	#include "playerstatusannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GFactionServer* gfs=GFactionServer::GetInstance();
		if (status == _STATUS_ONGAME)
		{
			GUserFaction fplayer;
			PlayerFactionInfo_Re pfi_re(ERR_SUCCESS,PFactionInfoVector());
			int cur_gid=-1;
			for (size_t i=0;i<playerstatus_list.size();i++)
			{
				OnlinePlayerStatus& ops=playerstatus_list[i];
				//get player faction_id from database,Add playerFactionInfo to pfi_re
				//DEBUG_PRINT("gfaction::statusannounce: roleid=%d\n",ops.roleid);
				if ( !Factiondb::GetInstance()->FindUserFaction(ops.roleid,fplayer,REASON_LOGIN) )
				{
					fplayer.fid=_FACTION_ID_INVALID;
					fplayer.role=_R_UNMEMBER;
				}
				else
				{
					//add users info to pfi_re
					if (cur_gid!=ops.gid)
					{
						if (pfi_re.faction_info.size()>0)
						{
							GProviderServer::GetInstance()->DispatchProtocol(cur_gid,pfi_re);
							pfi_re.faction_info.GetVector().clear();
						}
						cur_gid=ops.gid;
					}
					pfi_re.faction_info.add(PFactionInfo(ops.roleid,fplayer.fid,fplayer.role));
				}
				//add player to map
				gfs->RegisterPlayer(ops,fplayer.fid,fplayer.role);
			}
			// send PlayerFactionInfo_Re to remote server
			GProviderServer::GetInstance()->DispatchProtocol(cur_gid,pfi_re);
		}
		else
		{
			for (size_t i=0;i<playerstatus_list.size();i++)
			{
				OnlinePlayerStatus& ops=playerstatus_list[i];
				gfs->UnRegisterPlayer(ops.roleid);
			}
		}
	}
};

};

#endif
