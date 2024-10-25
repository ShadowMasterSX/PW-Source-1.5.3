
#ifndef __GNET_GETUICONFIG_HPP
#define __GNET_GETUICONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "getuiconfig_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class GetUIConfig : public GNET::Protocol
{
	#include "getuiconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->rbcache.Lock();
		GRoleBase* grb=dsm->rbcache.GetDirectly(roleid);
		GetUIConfig_Re	getui_re(ERR_SUCCESS,roleid,localsid,Octets());
		if (grb!=NULL) 
		{
			getui_re.ui_config = grb->config_data;
			manager->Send(sid,getui_re);
		}
		else
		{
			GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(roleid));
			rpc->response_type = _RESPONSE_GET_GUI;
			rpc->need_send2client = true;
			rpc->save_roleid = roleid;
			rpc->save_link_sid = sid;
			rpc->save_localsid = localsid;
			if (!GameDBClient::GetInstance()->SendProtocol(rpc))
			{
				getui_re.result=ERR_COMMUNICATION;
				manager->Send(sid,getui_re);
			}
		}
		dsm->rbcache.UnLock();
	}
};

};

#endif
