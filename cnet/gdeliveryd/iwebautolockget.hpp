
#ifndef __GNET_IWEBAUTOLOCKGET_HPP
#define __GNET_IWEBAUTOLOCKGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gpair"

#include "gamedbclient.hpp"
#include "iwebautolockget_re.hpp"

namespace GNET
{

class IWebAutolockGet : public GNET::Protocol
{
	#include "iwebautolockget"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if(NULL==pinfo || !pinfo->rolelist.IsRoleListInitialed()) 
		{
			DBAutolockGetArg arg;
			arg.userid = userid;

			DBAutolockGet* rpc = (DBAutolockGet*) Rpc::Call(RPC_DBAUTOLOCKGET,
					arg);
			rpc->save_sid = sid;
			rpc->save_tid = tid;

			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		else
		{
			int gmsettime = pinfo->autolock.GetValue(LOCKSET_TIME_GM);
			int gmlocktime = pinfo->autolock.GetValue(LOCKTIME_GM);
			manager->Send(sid, IWebAutolockGet_Re(tid, ERR_SUCCESS, userid, gmsettime, gmlocktime));
		}
	}
};

};

#endif
