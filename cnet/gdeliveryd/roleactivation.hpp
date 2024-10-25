
#ifndef __GNET_ROLEACTIVATION_HPP
#define __GNET_ROLEACTIVATION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbgametalkrolestatus.hrp"
#include "gamedbclient.hpp"
#include "gametalkmanager.h"
#include "mapuser.h"
#include "gametalkclient.hpp"

namespace GNET
{

class RoleActivation : public GNET::Protocol
{
	#include "roleactivation"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(_TryCache()) return;

		DBGameTalkRoleStatusArg arg;
		arg.roleid = (int)roleid;
		DBGameTalkRoleStatus *rpc = (DBGameTalkRoleStatus *)Rpc::Call(RPC_DBGAMETALKROLESTATUS, arg);
		rpc->operation = operation;
		if(!GameDBClient::GetInstance()->SendProtocol(rpc)) {
			Log::log(LOG_ERR, "RoleActivation send DBGameTalkRoleStatus failed.");
		}
	}
private:
	bool _TryCache() {
		int userid = UidConverter::Instance().Roleid2Uid((int)roleid);
		if(userid == 0) return false; 

		UserInfo *userinfo = UserContainer::GetInstance().FindUser(userid);
		if(!userinfo) return false;

		switch(userinfo->role_status[(int)roleid % MAX_ROLE_COUNT]) {
		case _ROLE_STATUS_READYDEL:
			GameTalkManager::GetInstance()->NotifyPreRemoveRole((int)roleid);
			break;
		case _ROLE_STATUS_MUSTDEL:
			GameTalkManager::GetInstance()->NotifyRemoveRole(userid, (int)roleid);
			break;
		default:
			GameTalkManager::GetInstance()->SendActiveRole((int)roleid, operation);
			break;
		}
		return true;
	}
};

};

#endif
