
#ifndef __GNET_PLAYERBASEINFO_HPP
#define __GNET_PLAYERBASEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
#include "playerbaseinfo_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class PlayerBaseInfo : public GNET::Protocol
{
	#include "playerbaseinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		cache_stamp stamp;
		PlayerBaseInfo_Re pbi_re(ERR_SUCCESS,roleid,localsid,GRoleBase());
		for (size_t i=0;i<playerlist.size();i++)
		{
			if(playerlist[i]<=0)
			{
				break;
			}
			if (dsm->rbcache.get(playerlist[i],pbi_re.player,stamp))
			{
				if (pbi_re.player.id!=(unsigned)roleid) 
				{
					pbi_re.player.config_data.clear();
					pbi_re.player.forbid.clear();
					pbi_re.player.help_states.clear();
				}
				manager->Send(sid,pbi_re);
			}
			else
			{
				GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(playerlist[i]));
				rpc->response_type = _RESPONSE_BASEINFO;
				rpc->need_send2client = true;
				rpc->save_roleid = roleid;
				rpc->save_link_sid = sid;
				rpc->save_localsid = localsid;
				if (!GameDBClient::GetInstance()->SendProtocol(rpc))
				{
					pbi_re.retcode=ERR_COMMUNICATION;
					manager->Send(sid,pbi_re);
					break;
				}
				break;
			}
		}
	}
};

};

#endif
