
#ifndef __GNET_VERIFYMASTER_HPP
#define __GNET_VERIFYMASTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class VerifyMaster : public GNET::Protocol
{
	#include "verifymaster"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("receive verifymater from AU");
		DBVerifyMaster* rpc=(DBVerifyMaster*)Rpc::Call(
					RPC_DBVERIFYMASTER,
					DBVerifyMasterArg(rolename, faction));
		GameDBClient::GetInstance()->SendProtocol( rpc );		

	}
};

};

#endif
