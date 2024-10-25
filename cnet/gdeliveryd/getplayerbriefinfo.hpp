
#ifndef __GNET_GETPLAYERBRIEFINFO_HPP
#define __GNET_GETPLAYERBRIEFINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "getplayerbriefinfo_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class GetPlayerBriefInfo : public GNET::Protocol
{
	#include "getplayerbriefinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->rbcache.Lock();
		GetPlayerBriefInfo_Re	getpbi_re(ERR_SUCCESS,roleid,localsid,PlayerBriefInfoVector(),reason);
		
		for (size_t i=0;i<playerlist.size()&&playerlist[i]>0;i++)
		{	
			GRoleBase* grb=dsm->rbcache.GetDirectly(playerlist[i]);
			if (grb!=NULL) 
			{
				getpbi_re.playerlist.add(PlayerBriefInfo(grb->id,grb->cls,grb->name,grb->gender));
			}
			else
			{
				GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(playerlist[i]));
				rpc->response_type = _RESPONSE_GET_PBI;
				rpc->need_send2client = true;
				rpc->save_roleid = roleid;
				rpc->save_link_sid = sid;
				rpc->save_localsid = localsid;
				rpc->save_reason = reason;
				if (!GameDBClient::GetInstance()->SendProtocol(rpc))
				{
					getpbi_re.retcode=ERR_COMMUNICATION;
				}
				break;
			}
		}
		if ((getpbi_re.retcode==ERR_SUCCESS && getpbi_re.playerlist.size()!=0) || getpbi_re.retcode!=ERR_SUCCESS)
			manager->Send(sid,getpbi_re);
		dsm->rbcache.UnLock();
	}
};

};

#endif
