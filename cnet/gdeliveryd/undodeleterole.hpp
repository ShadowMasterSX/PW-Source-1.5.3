
#ifndef __GNET_UNDODELETEROLE_HPP
#define __GNET_UNDODELETEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "undodeleterole_re.hpp"
#include "dbundodeleterole.hrp"
#include "mapuser.h"
namespace GNET
{

class UndoDeleteRole : public GNET::Protocol
{
	#include "undodeleterole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();

		int userid = UidConverter::Instance().Roleid2Uid(roleid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo* userinfo=UserContainer::GetInstance().FindUser(userid,sid,localsid);
		if (NULL==userinfo || userinfo->linksid!=sid || userinfo->localsid!=localsid)
			return;
		if(!userinfo->CheckUndoDeleteRole(roleid))
			return;
		
		if (userinfo->gameid==_GAMESERVER_ID_INVALID && userinfo->rolelist.IsRoleExist(roleid))
		{
			DBUndoDeleteRole* rpc=(DBUndoDeleteRole*) Rpc::Call(RPC_DBUNDODELETEROLE,DBUndoDeleteRoleArg(roleid));
			rpc->userid = userid;
			if (!GameDBClient::GetInstance()->SendProtocol(rpc))
			{
				dsm->Send(sid,UndoDeleteRole_Re(ERR_UNDODELROLE,roleid,localsid));
			}
		}
		else
		{
			dsm->Send(sid,UndoDeleteRole_Re(ERR_UNDODELROLE,roleid,localsid));
		}
	}
};

};

#endif
