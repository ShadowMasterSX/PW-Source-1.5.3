
#ifndef __GNET_GMFORBIDROLE_HPP
#define __GNET_GMFORBIDROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "putroleforbid.hrp"
#include "gamedbclient.hpp"

#include "forbid.hxx"
#include "protocolexecutor.h"
#include "gmkickoutrole.hpp"
#include "gmshutuprole.hpp"
#include "mapforbid.h"
namespace GNET
{

class GMForbidRole : public GNET::Protocol
{
	#include "gmforbidrole"
	void Send2DB()
	{
		RoleForbidPair arg;
		arg.key=RoleId(dstroleid);
		arg.value.add(GRoleForbid(fbd_type,forbid_time,time(NULL),reason));
		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_PUTROLEFORBID,&arg));
	}
	void HandleForbid(Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		switch (fbd_type)
		{
			case Forbid::FBD_FORBID_LOGIN:
			{	
				ProtocolExecutor* task=new ProtocolExecutor( dsm,sid,
						new GMKickoutRole(gmroleid,localsid,dstroleid,forbid_time,reason) );
				Thread::Pool::AddTask(task);
				break;
			}
			case Forbid::FBD_FORBID_TALK:
			{
				ProtocolExecutor* task=new ProtocolExecutor( dsm,sid,
						new GMShutupRole(gmroleid,localsid,dstroleid,forbid_time,reason) );
				Thread::Pool::AddTask(task);
				break;
			}
			case Forbid::FBD_FORBID_TRADE:
			{
				//add to forbidtrade map
				GRoleForbid	forbid(fbd_type,forbid_time,time(NULL),reason); 
				ForbidTrade::GetInstance().SetForbidTrade( dstroleid, forbid );
				break;
			}
			case Forbid::FBD_FORBID_SELL:
				break;	
		}
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		//DEBUG_PRINT("gdeliveryd::gmforbidrole: received. gmroleid=%d,forbidtype=%d,forbidroleid=%d,forbidtime=%d,reasone=%.*s\n",gmroleid,fbd_type,dstroleid,forbid_time,reason.size(),(char*)reason.begin());
		//put forbid record into GameDB
		Send2DB();
		//handle forbid
		HandleForbid(sid);
	}
};

};

#endif
