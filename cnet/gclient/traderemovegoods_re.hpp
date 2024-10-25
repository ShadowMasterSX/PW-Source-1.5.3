
#ifndef __GNET_TRADEREMOVEGOODS_RE_HPP
#define __GNET_TRADEREMOVEGOODS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"

#include "glinkclient.hpp"
#include "tradechoice.h"
namespace GNET
{

class TradeRemoveGoods_Re : public GNET::Protocol
{
	#include "traderemovegoods_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch (retcode)
		{
			case ERR_TRADE_INVALID_TRADER:
				DEBUG_PRINT("Remove goods failed. You are invalid trader.\n");
				break;
			case ERR_TRADE_RMVGOODS:
				DEBUG_PRINT("Remove goods failed. You do not have exchanged the object(id=%d,money=%d),or not enough\n",goods.id,money);
				TradeChoice(tid,owner_roleid,manager,sid);
				break;
			case ERR_SUCCESS:
				if (owner_roleid == GLinkClient::GetInstance()->roleid)
				{
					DEBUG_PRINT("You remove goods(id=%d,pos=%d,count=%d,money=%d) successfully.\n",goods.id,goods.pos,goods.count,money);
					TradeChoice(tid,owner_roleid,manager,sid);
				}
				else
				{
					DEBUG_PRINT("Your partner(roleid=%d) removes goods(id=%d,pos=%d,count=%d,money=%d).\n",owner_roleid,goods.id,goods.pos,goods.count,money);
				}	
				break;
		}

	}
};

};

#endif
