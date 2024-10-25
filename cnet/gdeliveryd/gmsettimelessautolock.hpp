
#ifndef __GNET_GMSETTIMELESSAUTOLOCK_HPP
#define __GNET_GMSETTIMELESSAUTOLOCK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GMSetTimelessAutoLock : public GNET::Protocol
{
	#include "gmsettimelessautolock"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(dstroleid);
		if(NULL==pinfo) 
			return;
		if(pinfo->roleid!=dstroleid||pinfo->user->status!=_STATUS_ONGAME)
			return;
		if(pinfo->user->GMSetLocktime(enable ? -1 : 0)==0)
		{
			DEBUG_PRINT("gmsettimelessautolock: gmroleid=%d, dstroleid=%d, enable=%d\n", gmroleid, dstroleid, enable);
			DBAutolockSetArg arg;
			arg.userid = pinfo->userid;
			arg.autolock = pinfo->user->autolock.GetList();
			DBAutolockSet * rpc = (DBAutolockSet*) Rpc::Call( RPC_DBAUTOLOCKSET,arg);
			rpc->reason = DBAutolockSet::REASON_GM;
			rpc->save_timeout = 0;
			rpc->save_dstroleid = dstroleid;
			rpc->save_sid = sid;
			rpc->save_localsid = localsid;
			rpc->save_tid = 0;

			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		else
			manager->Send(sid,GMSetTimelessAutoLock_Re(-1,localsid,dstroleid));
	}
};

};

#endif
