
#ifndef __GNET_DELETEROLE_HPP
#define __GNET_DELETEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "delrole.hrp"
#include "deleterole_re.hpp"
#include "dbdeleterole.hrp"
#include "mapuser.h"
namespace GNET
{

class DeleteRole : public GNET::Protocol
{
	#include "deleterole"
	/* this will not delete the role in DB, only mark the role should be delete*/
	void Mark_Delete(GDeliveryServer* dsm, Manager::Session::ID sid)  
	{
		int userid = UidConverter::Instance().Roleid2Uid(roleid);
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo* userinfo=UserContainer::GetInstance().FindUser(userid);
		if (!userinfo || userinfo->linksid!=sid || userinfo->localsid!=localsid) 
			return;
		if(!userinfo->CheckDeleteRole(roleid))
			return;
		
		DelRole* rpc=(DelRole*) Rpc::Call(RPC_DELROLE,RoleId(roleid));
		rpc->save_link_sid=sid;
		rpc->save_localsid=localsid;
		if (!GameDBClient::GetInstance()->SendProtocol(rpc))
		{
			dsm->Send(sid,DeleteRole_Re(ERR_DELETEROLE,roleid,localsid));
		}
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		
		if(dsm->IsCentralDS()) {
			Log::log(LOG_ERR, "roleid %d try to delete role on Central Delivery Server, refuse him!", roleid);
			return;
		}

		/* Mark the delete flag only*/
		Mark_Delete(dsm,sid);
	}
};

};

#endif
