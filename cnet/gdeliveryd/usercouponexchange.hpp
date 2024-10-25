
#ifndef __GNET_USERCOUPONEXCHANGE_HPP
#define __GNET_USERCOUPONEXCHANGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "couponexchange.hrp"
#include "gauthclient.hpp"

namespace GNET
{

class UserCouponExchange : public GNET::Protocol
{
	#include "usercouponexchange"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
		if(userinfo == NULL)
			return;

		if(GAuthClient::GetInstance()->GetVersion() == 1)
		{
			//简单验证一下,点券元宝兑换比例100:1
			if(coupon_number < 100 || coupon_number%100 != 0) return;
			int cash_number = coupon_number;

			CouponExchange * rpc = (CouponExchange *)Rpc::Call(RPC_COUPONEXCHANGE,CouponExchangeArg(userid,coupon_number,cash_number,Timer::GetTime()));
			rpc->save_linksid = userinfo->linksid;
			rpc->save_localsid = userinfo->localsid;
			GAuthClient::GetInstance()->SendProtocol(rpc);
		}
	}
};

};

#endif
