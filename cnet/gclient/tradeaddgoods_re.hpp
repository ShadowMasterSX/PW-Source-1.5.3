
#ifndef __GNET_TRADEADDGOODS_RE_HPP
#define __GNET_TRADEADDGOODS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"

#include "glinkclient.hpp"
#include "tradechoice.h"
namespace GNET
{

class TradeAddGoods_Re : public GNET::Protocol
{
	#include "tradeaddgoods_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch (retcode)
		{
			case ERR_TRADE_INVALID_TRADER:
				DEBUG_PRINT("Add goods failed. You are invalid trader.\n");
				break;
			case ERR_TRADE_ADDGOODS:
				DEBUG_PRINT("Add goods failed. You do not have the object(id=%d,money=%d),or not enough\n",goods.id,money);
				TradeChoice(tid,owner_roleid,manager,sid);
				break;
			case ERR_SUCCESS:
				if (owner_roleid == GLinkClient::GetInstance()->roleid)
				{
					DEBUG_PRINT("You add goods(id=%d,pos=%d,count=%d,max_count=%d,property=%.*s,money=%d) successfully.\n",goods.id,goods.pos,goods.count,goods.max_count,goods.data.size(),(char*)goods.data.begin(),money);
					TradeChoice(tid,owner_roleid,manager,sid);
				}
				else
				{
					DEBUG_PRINT("Your partner(roleid=%d) add goods(id=%d,pos=%d,count=%d,max_count=%d,property=%.*s,money=%d) successfully.\n",owner_roleid,goods.id,goods.pos,goods.count,goods.max_count,goods.data.size(),(char*)goods.data.begin(),money);
				}	
				break;
		}
	}
};

};

#endif
