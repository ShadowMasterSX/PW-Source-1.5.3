
#ifndef __GNET_DELROLEANNOUNCE_HPP
#define __GNET_DELROLEANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "accountdelrole.hrp"
#include "groledbclient.hpp"
#include "gdeliveryserver.hpp"
#include "uniquenameclient.hpp"
namespace GNET
{

class DelRoleAnnounce : public GNET::Protocol
{
	#include "delroleannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if ( !UniqueNameClient::GetInstance()->IsConnect() )
			return;
		for (size_t i=0;i<rolelist.size();i++)
		{
			DEBUG_PRINT("\tdelroleannounce::Try to delrole %d\n",rolelist[i]);
			DBDeleteRole* rpc=(DBDeleteRole*) Rpc::Call(RPC_DBDELETEROLE,DBDeleteRoleArg(rolelist[i],false));//do not send result to client
			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
	}
};

};

#endif
