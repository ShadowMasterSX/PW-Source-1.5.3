
#ifndef __GNET_GTRADEDISCARD_HPP
#define __GNET_GTRADEDISCARD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "trade.h"
#include "gtradeend.hpp"
#include "tradesave.hrp"
#include "mapuser.h"
namespace GNET
{

class GTradeDiscard : public GNET::Protocol
{
	#include "gtradediscard"
	void SendResult(GNET::Transaction* t,int retcode)
	{
		//send result to both traders
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Trader* r;
		if((r=t->GetAlice())!=NULL)
			dsm->Send(r->linksid,TradeDiscard_Re(retcode,tid,TRADER_SERVER,r->roleid,r->localsid));
		if((r=t->GetBob())!=NULL)
			dsm->Send(r->linksid,TradeDiscard_Re(retcode,tid,TRADER_SERVER,r->roleid,r->localsid));
	}
	bool Write2DB(GNET::Transaction* t,bool& blNeedSave)
	{
		/* write to DB */
		TradeSaveArg arg(t->GetAlice()->roleid,t->GetBob()->roleid);
		t->GetExchgResult(arg.goods1, &arg.money1, arg.goods2, &arg.money2,blNeedSave);
		if (!blNeedSave) return false;
		Log::formatlog("trade_debug","gdelivery::tradediscard:Send Trade Result to DB. (%d)'s exchg_obj.size=%d,"
			       "money=%d, (%d)'s exchg_obj.size=%d,money=%d,tid=%d\n",
			       t->GetAlice()->roleid,arg.goods1.size(),arg.money1,t->GetBob()->roleid,
			       arg.goods2.size(),arg.money2,t->GetTid());
		TradeSave* rpc = (TradeSave*) Rpc::Call(RPC_TRADESAVE,&arg);
		rpc->tid=tid;
		rpc->cause=PROTOCOL_TRADEDISCARD;
		rpc->cause_param = TRADER_SERVER;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::Transaction* trans=GNET::Transaction::GetTransaction(tid);
		if (trans == NULL) return;
		
		if (trans->Discard()==ERR_SUCCESS)
		{
			bool blNeedSave=true;
			if (!Write2DB(trans,blNeedSave))
			{
				if (blNeedSave)
					SendResult(trans,ERR_TRADE_DB_FAILURE);
				else
					SendResult(trans,ERR_SUCCESS);
				trans->Destroy();
			}
		}
	}
};

};

#endif
