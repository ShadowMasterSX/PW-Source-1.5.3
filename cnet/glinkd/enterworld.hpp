
#ifndef __GNET_ENTERWORLD_HPP
#define __GNET_ENTERWORLD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "gproviderserver.hpp"
namespace GNET
{

class EnterWorld : public GNET::Protocol
{
	#include "enterworld"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("glinkd:: receive enterworld from client. send to delivery. roleid=%d,localsid=%d\n",roleid,sid);
		GLinkServer* lsm=GLinkServer::GetInstance();
		GLinkServer::RoleMap::iterator it=lsm->roleinfomap.find(roleid);
		if (it==lsm->roleinfomap.end())
		{ 
			DEBUG_PRINT("LinkServer::EnterWorld_1: Active Close Session(%d)(roleid=%d)\n",sid,roleid);
			lsm->Close(sid);
			lsm->ActiveCloseSession(sid);
			return;
		}
		if ((*it).second.sid!=sid || (*it).second.roleid!=roleid)
		{ 
			DEBUG_PRINT("LinkServer::EnterWorld_2: Active Close Session(%d)(roleid=%d)\n",sid,roleid);
			lsm->Close(sid); 
			lsm->ActiveCloseSession(sid);
			return;
		}
		(*it).second.status = _STATUS_ONGAME;
		this->localsid=sid;
		this->provider_link_id=GProviderServer::GetProviderServerID();
		if ( GDeliveryClient::GetInstance()->SendProtocol(this) )
		{
			if (GDeliveryClient::GetInstance()->IsPhoneLink())
			{
				lsm->ChangeState(sid,&state_GPhoneReceive);
			}
			else
			{
				lsm->ChangeState(sid,&state_GDataExchgServer);
			}
			lsm->SetAliveTime(sid, _CLIENT_TTL);
		}
		else
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
