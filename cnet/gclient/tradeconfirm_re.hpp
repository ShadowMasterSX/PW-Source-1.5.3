
#ifndef __GNET_TRADECONFIRM_RE_HPP
#define __GNET_TRADECONFIRM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "tradechoice.h"
namespace GNET
{

class TradeConfirm_Re : public GNET::Protocol
{
	#include "tradeconfirm_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch (retcode)
		{
			case ERR_TRADE_INVALID_TRADER:
				DEBUG_PRINT("Trade confirm failed. you are invalid trader.\n");
				break;
			case ERR_TRADE_CONFIRM_FAIL:
				DEBUG_PRINT("Trade confirm failed. you are not allowed to confirm,submit first and do not confirm twice.\n");
				TradeChoice(tid,roleid,manager,sid);
				break;
			case ERR_TRADE_DB_FAILURE:
				DEBUG_PRINT("Trade confirm failed. DB error.\n");
				break;
			case ERR_TRADE_HALFDONE:
				if (confirm_roleid == GLinkClient::GetInstance()->roleid)
					DEBUG_PRINT("Trade confirm successfully. wait your partner to confirm.\n");
				else
					DEBUG_PRINT("Your partner confirm successfully. wait you to confirm (60 seconds).\n");
				TradeChoice(tid,roleid,manager,sid);
				break;
			case ERR_TRADE_DONE:
				if (confirm_roleid == GLinkClient::GetInstance()->roleid)
					DEBUG_PRINT("Trade confirm successfully.\n");
				else
					DEBUG_PRINT("Your partner confirm successfully.\n");
				break;
		}
	}
};

};

#endif
