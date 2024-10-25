
#ifndef __GNET_SETHELPSTATES_HPP
#define __GNET_SETHELPSTATES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "putrolebase.hrp"

#include "gdeliveryserver.hpp"
#include "sethelpstates_re.hpp"

#include "getrolebase.hrp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class SetHelpStates : public GNET::Protocol
{
	#include "sethelpstates"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("gdelivery::SetHelpStates: roleid=%d, localsid=%d\n",roleid,localsid);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->rbcache.Lock();
		GRoleBase* grb=dsm->rbcache.GetDirectly(roleid);
		if (grb!=NULL) 
		{
			grb->help_states.swap(help_states);
			dsm->Send(sid,SetHelpStates_Re(ERR_SUCCESS,roleid,localsid));
			//set rolebase to db
			RoleBasePair rbp;
			rbp.key.id=roleid;
			rbp.value=*grb;
			GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_PUTROLEBASE,&rbp));
		}
		else
		{
			//get GRoleBase from DB
			GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(roleid));
			rpc->data.swap(help_states);
			rpc->response_type= _RESPONSE_SET_HS;
			rpc->need_send2client = true;
			rpc->save_roleid = roleid;
			rpc->save_link_sid = sid;
			rpc->save_localsid = localsid;
			if (!GameDBClient::GetInstance()->SendProtocol(rpc))
				dsm->Send(sid,SetHelpStates_Re(ERR_COMMUNICATION,roleid,localsid));
		}
		dsm->rbcache.UnLock();

	}
};

};

#endif
