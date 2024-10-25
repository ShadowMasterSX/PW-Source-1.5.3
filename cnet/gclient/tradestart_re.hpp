
#ifndef __GNET_TRADESTART_RE_HPP
#define __GNET_TRADESTART_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
	#include "tradechoice.h"
#include "glinkclient.hpp"
namespace GNET
{

class TradeStart_Re : public GNET::Protocol
{
	#include "tradestart_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch (retcode)
		{
			case ERR_SUCCESS:
				DEBUG_PRINT("Trader %d agreed to trade with you. tid=%d\n",partner_roleid,tid);
				GLinkClient::GetInstance()->tid=tid;
				TradeChoice(tid,GLinkClient::GetInstance()->roleid,manager,sid);
				break;
			case ERR_TRADE_REFUSE:
				DEBUG_PRINT("Trader %d refused to trade with you.\n",partner_roleid);
				break;
			case ERR_TRADE_PARTNER_OFFLINE:
				DEBUG_PRINT("Trader %d is offline.\n",partner_roleid);
				break;
			case ERR_TRADE_INVALID_TRADER:
				DEBUG_PRINT("Trader %d is in different gameserver.\n",partner_roleid);
				break;	
			default:
				DEBUG_PRINT("Trade established failed.\n",partner_roleid);
				break;
		}
	}
};

};

#endif
