
#ifndef __GNET_GTRADESTART_RE_HPP
#define __GNET_GTRADESTART_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tradeinventory.hrp"
#include "gamedbclient.hpp"
#include "trade.h"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "gtradeend.hpp"
namespace GNET
{

class GTradeStart_Re : public GNET::Protocol
{
	#include "gtradestart_re"
	void SendError(GNET::Transaction* t,int errcode)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Trader* r;
		if((r=t->GetAlice())!=NULL)
			dsm->Send(r->linksid,TradeStart_Re(errcode,_TRADE_ID_INVALID,TRADER_SERVER,r->roleid,r->localsid));
		if((r=t->GetBob())!=NULL)
			dsm->Send(r->linksid,TradeStart_Re(errcode,_TRADE_ID_INVALID,TRADER_SERVER,r->roleid,r->localsid));
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::Transaction* t = GNET::Transaction::GetTransaction(tid);
		if (t == NULL) return;
		
		if (retcode == ERR_SUCCESS)
		{
			DEBUG_PRINT("gtradestart_re::Trader %d can join in the transaction(tid=%d).\n",roleid,tid);
			t->OnDestroy = GTradeEnd::OnTransactionDestroy;  //set callback function
			int retcode=t->AttachTrader(roleid);
			if (retcode != ERR_TRADE_ATTACH_HALF && retcode != ERR_TRADE_ATTACH_DONE)
			{
				Log::log(LOG_ERR,"gtradestart_re::Attach trader failed. retcode=%d(tid=%d)\n",retcode,tid);
				SendError(t,retcode);
				t->Destroy();
			}
			else if (retcode == ERR_TRADE_ATTACH_DONE)
			{
				//get traders' possession from GameDB	
				TradeInventory* rpc=(TradeInventory*) Rpc::Call(RPC_TRADEINVENTORY,
					TradeInventoryArg(t->GetAlice()->roleid,t->GetBob()->roleid));
				rpc->tid = tid;
				rpc->localsid1=t->GetAlice()->localsid;
				rpc->localsid2=t->GetBob()->localsid;
				if (!GameDBClient::GetInstance()->SendProtocol(rpc))
				{
					Log::log(LOG_ERR,"gtradestart_re::Send to GameDB failed.(tid=%d)\n",tid);
					SendError(t,ERR_TRADE_DB_FAILURE);
					t->Destroy();
				}
			}
		}
		else
		{
			DEBUG_PRINT("gtradestart_re::Trader %d can not join in the transaction(tid=%d).\n",roleid,tid);
			SendError(t,retcode);
			t->Destroy();
		}	
	}
};

};

#endif
