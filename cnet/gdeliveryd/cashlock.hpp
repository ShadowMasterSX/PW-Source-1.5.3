
#ifndef __GNET_CASHLOCK_HPP
#define __GNET_CASHLOCK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "cashlock_re.hpp"

namespace GNET
{

class CashLock : public GNET::Protocol
{
	#include "cashlock"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
                DEBUG_PRINT("cashlock: receive. roleid=%d,cash_pwd=%d,action=%d",userid,cash_password.size(),lock);
		userid = UidConverter::Instance().Roleid2Uid(userid);
		if(!userid)
			return;

		CashLock_Re res;
		res.localsid = localsid;
		if (!lock) // unlock
		{
			MD5Hash ctx;
			ctx.Update(cash_password);
			ctx.Final(cash_password);
			res.retcode = StockExchange::Instance()->CashUnlock(userid, cash_password);
		}
		else  // lock
		{
			res.retcode = StockExchange::Instance()->CashLock(userid); 
		}

		GDeliveryServer::GetInstance()->Send(sid, res);
	}
};

};

#endif
