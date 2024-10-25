
#ifndef __GNET_GTRADEEND_HPP
#define __GNET_GTRADEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "trade.h"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "tradeend.hpp"
#include "mapuser.h"
namespace GNET
{

class GTradeEnd : public GNET::Protocol
{
	#include "gtradeend"
	static void OnTransactionDestroy(unsigned int tid,bool bl_timeout,bool blModified1,bool blModified2)
	{
		DEBUG_PRINT("******Enter OnTransactionDestroy.... tid=%d,timeout=%d,AModified=%d,BModified=%d*******\n",
			tid, bl_timeout,blModified1,blModified2);
		GNET::Transaction* t=GNET::Transaction::GetTransaction(tid);
		if (t==NULL) return;	
		GProviderServer::GetInstance()->DispatchProtocol(t->gs_id,GTradeEnd(tid,t->GetAlice()->roleid,blModified1,
			t->GetBob()->roleid,blModified2));
		//Send TradeEnd to player
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Trader* r;
		char cause = bl_timeout?_TRADE_END_TIMEOUT:_TRADE_END_NORMAL;
		if((r=t->GetAlice())!=NULL)
			dsm->Send(r->linksid,TradeEnd(tid,cause,r->roleid,r->localsid));
		if((r=t->GetBob())!=NULL)
			dsm->Send(r->linksid,TradeEnd(tid,cause,r->roleid,r->localsid));
		PollIO::WakeUp();
		return;
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
