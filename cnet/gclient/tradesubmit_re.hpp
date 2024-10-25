
#ifndef __GNET_TRADESUBMIT_RE_HPP
#define __GNET_TRADESUBMIT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"

#include "glinkclient.hpp"
#include "tradechoice.h"
namespace GNET
{

class TradeSubmit_Re : public GNET::Protocol
{
	#include "tradesubmit_re"
	/*
	void DisplayGoods(GRoleInventoryVector& goodslist,unsigned int money)
	{
		printf("\ntrader(id:%d)\n",submit_roleid);
		printf("\tExchange Obj:\n");
		for (size_t i=0;i<goodslist.size();i++)
            printf("\tid=%3d  pos=%3d  count=%3d max_count=%3d property=%.*s\n",goodslist[i].id,goodslist[i].pos,goodslist[i].count,goodslist[i].max_count,goodslist[i].data.size(),(char*)goodslist[i].data.begin());			
		printf("\tmoney=%d\n",money);
	}
	*/
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		
		// TODO
	
		switch (retcode)
		{
			case ERR_TRADE_INVALID_TRADER:
				DEBUG_PRINT("Submit goods failed. You are invalid trader.\n");
				break;
			case ERR_TRADE_SUBMIT_FAIL:
				DEBUG_PRINT("Submit goods failed. You have already submit or confirm\n");
				TradeChoice(tid,submit_roleid,manager,sid);
				break;
			case ERR_TRADE_READY_HALF:
				if (submit_roleid == GLinkClient::GetInstance()->roleid)
				{
					DEBUG_PRINT("You submit goods successfully. wait your partner to submit\n");
					//DisplayGoods(exchg_goods,exchg_money);
					TradeChoice(tid,submit_roleid,manager,sid);
				}
				else
				{
					DEBUG_PRINT("Your partner(roleid=%d) submit goods. wait you to submit\n",submit_roleid);
					//DisplayGoods(exchg_goods,exchg_money);
				}	
				break;
			case ERR_TRADE_READY:
				if (submit_roleid == GLinkClient::GetInstance()->roleid)
				{
					DEBUG_PRINT("You submit goods successfully. \n");
					//DisplayGoods(exchg_goods,exchg_money);
					TradeChoice(tid,submit_roleid,manager,sid);
				}
				else
				{
					DEBUG_PRINT("Your partner(roleid=%d) submit goods.\n",submit_roleid);
					//DisplayGoods(exchg_goods,exchg_money);
				}	
				break;
		}
	
	}
};

};

#endif
