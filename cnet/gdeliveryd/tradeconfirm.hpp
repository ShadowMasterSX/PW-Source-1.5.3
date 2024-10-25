
#ifndef __GNET_TRADECONFIRM_HPP
#define __GNET_TRADECONFIRM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "trade.h"
#include "tradeconfirm_re.hpp"
#include "tradesave.hrp"
#include "gamedbclient.hpp"
#include "gtradeend.hpp"
#include "mapuser.h"
namespace GNET
{

class TradeConfirm : public GNET::Protocol
{
	#include "tradeconfirm"
	void SendResult(GNET::Transaction* t,int retcode)
	{
		//send result to both traders
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Trader* r;
		if((r=t->GetAlice())!=NULL)
			dsm->Send(r->linksid,TradeConfirm_Re(retcode,tid,roleid,r->roleid,r->localsid));
		if((r=t->GetBob())!=NULL)
			dsm->Send(r->linksid,TradeConfirm_Re(retcode,tid,roleid,r->roleid,r->localsid));
	}
	bool Write2DB(GNET::Transaction* t,bool& blNeedSave)
	{
		/* write to DB */
		TradeSaveArg arg(t->GetAlice()->roleid,t->GetBob()->roleid);
		t->GetExchgResult(arg.goods1, &arg.money1, arg.goods2, &arg.money2,blNeedSave);
		if (!blNeedSave) return false;
		Log::formatlog("trade_debug","confirmed. %d:exchg_obj.size=%d,money=%d %d:exchg_obj.size=%d,money=%d tid=%d",
			t->GetAlice()->roleid,arg.goods1.size(),arg.money1,t->GetBob()->roleid,arg.goods2.size(),
			arg.money2,t->GetTid());
		TradeSave* rpc = (TradeSave*) Rpc::Call(RPC_TRADESAVE,&arg);
		rpc->tid=tid;
		rpc->cause=PROTOCOL_TRADECONFIRM;
		rpc->cause_param = roleid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::Transaction* t=GNET::Transaction::GetTransaction(tid);
		if (t == NULL) return;
		int retcode=t->Confirm(roleid);
		if (retcode != ERR_TRADE_DONE && retcode != ERR_TRADE_HALFDONE) 
		{
			LOG_TRACE("gdelivery::tradeconfirm: confirm error, roleid=%d,retcode=%d,tid=%d\n",roleid,retcode,tid);
			SendResult(t,retcode);
			if (retcode==ERR_TRADE_SPACE) t->Destroy();
		}	
		if (retcode == ERR_TRADE_HALFDONE)
		{
			SendResult(t,retcode);
			Log::formatlog("trade_debug","tradeconfirm: half confirmed. roleid=%d,tid=%d\n", roleid,tid);
		}
		if (retcode == ERR_TRADE_DONE)
		{
			/* write to DB  */
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
	}
};

};

#endif
