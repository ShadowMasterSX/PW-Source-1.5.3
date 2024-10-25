
#ifndef __GNET_CASHPASSWORDSET_HPP
#define __GNET_CASHPASSWORDSET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class CashPasswordSet : public GNET::Protocol
{
	#include "cashpasswordset"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("CashPasswordSet, userid=%d,password=%d", userid, cash_password.size());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(userid);
		if(!pinfo)
			return;
		userid = pinfo->userid;

		if(StockExchange::Instance()->IsCashLocked(userid))
		{
			DEBUG_PRINT("stock: try to set cash password when account is locked. userid=%d\n",userid);
			GDeliveryServer::GetInstance()->Send(sid, CashPasswordSet_Re(ERR_STOCK_CASHLOCKED, localsid));
			return;
		}

		DBSetCashPasswordArg arg;

		if(cash_password.size())
		{
			MD5Hash ctx;
			ctx.Update(cash_password);
			ctx.Final(arg.password);
		}

		arg.source = 0;
		arg.userid = userid; 

		DBSetCashPassword * rpc = (DBSetCashPassword*) Rpc::Call( RPC_DBSETCASHPASSWORD,arg);
		rpc->save_sid = sid;
		rpc->save_localsid = localsid;

		GameDBClient::GetInstance()->SendProtocol(rpc);
	}
};

};

#endif
