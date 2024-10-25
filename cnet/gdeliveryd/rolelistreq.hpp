
#ifndef __GNET_ROLELISTREQ_HPP
#define __GNET_ROLELISTREQ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "log.h"

#include "dbgametalkrolelist.hrp"


namespace GNET
{

class RoleListReq : public GNET::Protocol
{
	#include "rolelistreq"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DBGameTalkRoleListArg arg;
		arg.userid = (int)userid;
		DBGameTalkRoleList *rpc = (DBGameTalkRoleList *)Rpc::Call(RPC_DBGAMETALKROLELIST, arg);
		rpc->save_manager = manager; 
		rpc->save_sid = sid;
		if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
			Log::log(LOG_ERR, "RoleListReq send DBGameTalkRoleList failed.");
		};
	}
};

};

#endif
