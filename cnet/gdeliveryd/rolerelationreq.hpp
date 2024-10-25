
#ifndef __GNET_ROLERELATIONREQ_HPP
#define __GNET_ROLERELATIONREQ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "log.h"

#include "dbgametalkrolerelation.hrp"
#include "mapuser.h"

namespace GNET
{

class RoleRelationReq : public GNET::Protocol
{
	#include "rolerelationreq"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DBGameTalkRoleRelationArg arg;
		arg.roleid = (int)roleid;
		DBGameTalkRoleRelation *rpc = 
			(DBGameTalkRoleRelation *)Rpc::Call(RPC_DBGAMETALKROLERELATION, arg);
		rpc->userid = userid;
		rpc->save_manager = manager; 
		rpc->save_sid = sid;
		if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
			Log::log(LOG_ERR, "RoleRelationReq Send DBGameTalkRoleRelation failed.");
		}
	}
};

};

#endif
