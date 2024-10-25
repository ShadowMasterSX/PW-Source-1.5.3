
#ifndef __GNET_TRADEDISCARD_RE_HPP
#define __GNET_TRADEDISCARD_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "tradechoice.h"
namespace GNET
{

class TradeDiscard_Re : public GNET::Protocol
{
	#include "tradediscard_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch (retcode)
		{
			case ERR_SUCCESS:
				if (discard_roleid == roleid) 
				{
					DEBUG_PRINT("You discard the transcation\n");
				}
				else if (discard_roleid == 0)
				{
					DEBUG_PRINT("Gameserver discard the transcation,discard_roleid=%d\n",discard_roleid);
				}
				else	
				{
					DEBUG_PRINT("Your partner discard the transcation,discard_roleid=%d\n",discard_roleid);
				}
				break;
			case ERR_TRADE_DISCARDFAIL:
				DEBUG_PRINT("Discard transaction failed. You are not allowed to discard now.\n"); 
				TradeChoice(tid,roleid,manager,sid);
				break;
			case ERR_TRADE_DB_FAILURE:
				if (discard_roleid == roleid) 
				{
					DEBUG_PRINT("You discard the transcation,but write to DB failed.\n");
				}
				else if (discard_roleid == 0)
				{
					DEBUG_PRINT("Gameserver discard the transcation,but write to DB failed\n");
				}
				else	
				{
					DEBUG_PRINT("Your partner discard the transcation,but write to DB failed\n");
				}
				break;
			case ERR_TRADE_PARTNER_OFFLINE:
				DEBUG_PRINT("Your partner is offline. Transaction is discarded.\n");
				break;
		}
	}
};

};

#endif
