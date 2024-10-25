
#ifndef __GNET_TRADEDISCARD_HPP
#define __GNET_TRADEDISCARD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "trade.h"
#include "tradediscard_re.hpp"
#include "tradesave.hrp"
#include "gtradeend.hpp"
#include "gproviderserver.hpp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class TradeDiscard : public GNET::Protocol
{
	#include "tradediscard"
	void SendResult(GNET::Transaction* t,int retcode)
	{
		//send result to both traders
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Trader* r;
		if((r=t->GetAlice())!=NULL)
			dsm->Send(r->linksid,TradeDiscard_Re(retcode,tid,roleid,r->roleid,r->localsid));
		if((r=t->GetBob())!=NULL)
			dsm->Send(r->linksid,TradeDiscard_Re(retcode,tid,roleid,r->roleid,r->localsid));
	}
	bool Write2DB(GNET::Transaction* t,bool& blNeedSave)
	{
		/* write to DB */
		TradeSaveArg arg(t->GetAlice()->roleid,t->GetBob()->roleid);
		t->GetExchgResult(arg.goods1, &arg.money1, arg.goods2, &arg.money2, blNeedSave);
		if (!blNeedSave) 
			return false;
		Log::formatlog("trade_debug","tradediscard:Send Trade Result to DB. (%d)'s exchg_obj.size=%d, money=%d, (%d)'s exchg_obj.size=%d,money=%d,tid=%d\n",t->GetAlice()->roleid,arg.goods1.size(),arg.money1,t->GetBob()->roleid,arg.goods2.size(),arg.money2,t->GetTid());
		TradeSave* rpc = (TradeSave*) Rpc::Call(RPC_TRADESAVE,&arg);
		rpc->tid=tid;
		rpc->cause=PROTOCOL_TRADEDISCARD;
		rpc->cause_param = roleid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		GNET::Transaction* t=GNET::Transaction::GetTransaction(tid);
		if (t == NULL) return;
		Log::formatlog("trade_debug","tradediscard: roleid=%d,tid=%d\n",roleid,tid);
		int retcode=t->Discard();
		if (retcode == ERR_SUCCESS)
		{
			/* write to DB */
			bool blNeedSave=true;
			if (!Write2DB(t,blNeedSave))
			{
				if (blNeedSave)
					SendResult(t,ERR_TRADE_DB_FAILURE);
				else   	
					SendResult(t,ERR_SUCCESS);
				t->Destroy();
			}
		}
		else
		{
			//send error to roleid who want to discard
			dsm->Send(sid,TradeDiscard_Re(retcode,tid,roleid,roleid,localsid));
		}

	}
};

};

#endif
