
#ifndef __GNET_GETCUSTOMDATA_HPP
#define __GNET_GETCUSTOMDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "getcustomdata_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class GetCustomData : public GNET::Protocol
{
	#include "getcustomdata"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		
		dsm->rbcache.Lock();
		GetCustomData_Re	getcd_re(ERR_SUCCESS,roleid,localsid,0,Octets());
		for (size_t i=0;i<playerlist.size()&&playerlist[i]>0;i++)
		{
			GRoleBase* grb=dsm->rbcache.GetDirectly(playerlist[i]);
			if (grb!=NULL) 
			{
				getcd_re.cus_roleid=playerlist[i];
				getcd_re.customdata=grb->custom_data;
				manager->Send(sid,getcd_re);
			}
			else
			{
				GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(playerlist[i]));
				rpc->response_type = _RESPONSE_GET_CD;
				rpc->need_send2client = true;
				rpc->save_roleid = roleid;
				rpc->save_link_sid = sid;
				rpc->save_localsid = localsid;
				if (!GameDBClient::GetInstance()->SendProtocol(rpc))
				{
					getcd_re.retcode=ERR_COMMUNICATION;
					manager->Send(sid,getcd_re);
				}
				break;
			}
		}
		dsm->rbcache.UnLock();
	}
};

};

#endif
