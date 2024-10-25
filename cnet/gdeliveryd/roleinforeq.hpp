
#ifndef __GNET_ROLEINFOREQ_HPP
#define __GNET_ROLEINFOREQ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
#include "dbgametalkroleinfo.hrp"


namespace GNET
{

class RoleInfoReq : public GNET::Protocol
{
	#include "roleinforeq"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DBGameTalkRoleInfoArg arg;
		arg.roleid = (int)roleid;
		DBGameTalkRoleInfo *rpc = (DBGameTalkRoleInfo *)Rpc::Call(RPC_DBGAMETALKROLEINFO, arg);
		rpc->localuid = localuid;
		if(!GameDBClient::GetInstance()->SendProtocol(rpc))
			Log::log(LOG_ERR, "RoleInfoReq send DBGameTalkRoleInfo failed.");
	}
};

};

#endif
