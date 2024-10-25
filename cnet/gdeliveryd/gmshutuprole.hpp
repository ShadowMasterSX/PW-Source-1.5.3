
#ifndef __GNET_GMSHUTUPROLE_HPP
#define __GNET_GMSHUTUPROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "putroleforbid.hrp"
#include "gamedbclient.hpp"

#include "forbid.hxx"
#include "maplinkserver.h"
namespace GNET
{

class GMShutupRole : public GNET::Protocol
{
	#include "gmshutuprole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//put forbid record into GameDB
		RoleForbidPair arg;
		arg.key=RoleId(dstroleid);
		arg.value.add(GRoleForbid(Forbid::FBD_FORBID_TALK,forbid_time,time(NULL),reason));
		GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_PUTROLEFORBID,&arg));
		//announce to all linkserver expect source link
		LinkServer::GetInstance().BroadcastProtocol( this );
	}
};

};

#endif
