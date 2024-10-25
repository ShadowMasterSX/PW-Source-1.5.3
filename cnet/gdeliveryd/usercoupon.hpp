
#ifndef __GNET_USERCOUPON_HPP
#define __GNET_USERCOUPON_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "getusercoupon.hrp"
#include "gauthclient.hpp"

namespace GNET
{

class UserCoupon : public GNET::Protocol
{
	#include "usercoupon"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
		if(userinfo == NULL)
			return;

		if(GAuthClient::GetInstance()->GetVersion() == 1)
		{
			GetUserCoupon * rpc = (GetUserCoupon *)Rpc::Call(RPC_GETUSERCOUPON,GetUserCouponArg(userid));
			rpc->save_linksid = userinfo->linksid;
			rpc->save_localsid = userinfo->localsid;
			GAuthClient::GetInstance()->SendProtocol(rpc);
		}
	}
};

};

#endif
