
#ifndef __GNET_PLAYERBASEINFOCRC_HPP
#define __GNET_PLAYERBASEINFOCRC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "playerbaseinfocrc_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class PlayerBaseInfoCRC : public GNET::Protocol
{
	#include "playerbaseinfocrc"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		PlayerInfo* ui=UserContainer::GetInstance().FindRoleOnline(roleid);
		if (ui== NULL)
			return;
		PlayerBaseInfoCRC_Re pbi_crc_re(ERR_SUCCESS,roleid,localsid,IntVector()/*playerlist*/,IntVector()/*CRClist*/);
		dsm->rbcache.Lock();
		GRoleBase* grb=NULL;
		for (size_t i=0;i<playerlist.size();i++)
		{
			if(playerlist[i]<=0)
				break;
			grb=dsm->rbcache.GetDirectly(playerlist[i]);
			if (grb==NULL)
			{
				//get GRoleBase from DB
				GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(playerlist[i]));
				rpc->response_type= _RESPONSE_BASE_CRC;
				rpc->need_send2client = true;
				rpc->save_roleid = roleid;
				rpc->save_link_sid = sid;
				rpc->save_localsid = localsid;
				if (!GameDBClient::GetInstance()->SendProtocol(rpc))
					pbi_crc_re.result=ERR_COMMUNICATION;
				break;
			}
			else
			{
				pbi_crc_re.playerlist.add(playerlist[i]);
				pbi_crc_re.CRClist.add(grb->custom_stamp);
			}

		}
		manager->Send(sid,pbi_crc_re);
		dsm->rbcache.UnLock();
	}
};

};

#endif
