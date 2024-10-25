
#ifndef __GNET_AUTOLOCKSET_HPP
#define __GNET_AUTOLOCKSET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "dbautolockset.hrp"
#include "autolockset_re.hpp"


namespace GNET
{

class AutolockSet : public GNET::Protocol
{
	#include "autolockset"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);
		if(NULL==pinfo) 
			return;
		if(pinfo->roleid!=roleid||pinfo->linksid!=sid||pinfo->user->status!=_STATUS_ONGAME)
			return;
		if(pinfo->user->SetLocktime(timeout)==0)
		{
			DEBUG_PRINT("gdelivery::autolock: set locktime=%d,roleid=%d\n",timeout,roleid);
			DBAutolockSetArg arg;
			arg.userid = pinfo->userid;
			arg.autolock = pinfo->user->autolock.GetList();
			DBAutolockSet * rpc = (DBAutolockSet*) Rpc::Call( RPC_DBAUTOLOCKSET,arg);
			rpc->reason = DBAutolockSet::REASON_PLAYER;
			rpc->save_timeout = timeout;
			rpc->save_dstroleid = roleid;
			rpc->save_sid = sid;
			rpc->save_localsid = localsid;
			rpc->save_tid = 0;

			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		else
			manager->Send(sid,AutolockSet_Re(1,0,localsid));
	}
};

};

#endif
