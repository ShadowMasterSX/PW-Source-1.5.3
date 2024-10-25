
#ifndef __GNET_USERADDCASH_HPP
#define __GNET_USERADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "instantaddcash.hrp"
#include "gauthclient.hpp"

namespace GNET
{

class UserAddCash : public GNET::Protocol
{
	#include "useraddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
		if(userinfo == NULL)
			return;

		if(GAuthClient::GetInstance()->GetVersion() == 1)
		{
			//简单验证一下
			if(!cardnum.size() || !cardpasswd.size()) return;

			InstantAddCash * rpc = (InstantAddCash *)Rpc::Call(RPC_INSTANTADDCASH,InstantAddCashArg(userid,userinfo->ip,cardnum,cardpasswd));
			rpc->save_linksid = userinfo->linksid;
			rpc->save_localsid = userinfo->localsid;
			GAuthClient::GetInstance()->SendProtocol(rpc);
		}
	}
};

};

#endif
