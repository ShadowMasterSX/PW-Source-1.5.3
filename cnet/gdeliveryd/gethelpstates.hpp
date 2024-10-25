
#ifndef __GNET_GETHELPSTATES_HPP
#define __GNET_GETHELPSTATES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gethelpstates_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class GetHelpStates : public GNET::Protocol
{
	#include "gethelpstates"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->rbcache.Lock();
		GRoleBase* grb=dsm->rbcache.GetDirectly(roleid);
		GetHelpStates_Re	geths_re(ERR_SUCCESS,roleid,localsid,Octets());
		if (grb!=NULL) 
		{
			geths_re.help_states=grb->help_states;
			manager->Send(sid,geths_re);
		}
		else
		{
			GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(roleid));
			rpc->response_type = _RESPONSE_GET_HS;
			rpc->need_send2client = true;
			rpc->save_roleid = roleid;
			rpc->save_link_sid = sid;
			rpc->save_localsid = localsid;
			if (!GameDBClient::GetInstance()->SendProtocol(rpc))
			{
				geths_re.result=ERR_COMMUNICATION;
				manager->Send(sid,geths_re);
			}
		}
		dsm->rbcache.UnLock();
	}
};

};

#endif
