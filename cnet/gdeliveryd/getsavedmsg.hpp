
#ifndef __GNET_GETSAVEDMSG_HPP
#define __GNET_GETSAVEDMSG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GetSavedMsg : public GNET::Protocol
{
	#include "getsavedmsg"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GetMessage* rpc = (GetMessage*) Rpc::Call(RPC_GETMESSAGE,RoleId(roleid));
		rpc->linksid = sid;
		rpc->localsid = localsid;
		rpc->toGT = false;
		GameDBClient::GetInstance()->SendProtocol(rpc);
		return;
	}
};

};

#endif
