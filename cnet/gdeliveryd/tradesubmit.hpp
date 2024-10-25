
#ifndef __GNET_TRADESUBMIT_HPP
#define __GNET_TRADESUBMIT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"

#include "gdeliveryserver.hpp"
#include "trade.h"
#include "tradesubmit_re.hpp"
#include "mapuser.h"
namespace GNET
{

class TradeSubmit : public GNET::Protocol
{
	#include "tradesubmit"
	int problem_roleid;

	void SendResult(GNET::Transaction* t,int retcode)
	{
		//send result to both traders
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		int _roleid;
		_roleid= (retcode== ERR_TRADE_SPACE) ? problem_roleid : roleid;

		Trader* r;
		if((r=t->GetAlice())!=NULL)
			dsm->Send(r->linksid,TradeSubmit_Re(retcode,tid,_roleid,r->roleid,r->localsid));
		if((r=t->GetBob())!=NULL)
			dsm->Send(r->linksid,TradeSubmit_Re(retcode,tid,_roleid,r->roleid,r->localsid));
	}
	
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::Transaction* t=GNET::Transaction::GetTransaction(tid);
		if (t == NULL) return;

		int retcode=t->Submit(roleid, &problem_roleid);
		Log::formatlog("trade_debug","tradesubmit,rid=%d,A:%d,B:%d,retcode=%d,tid=%d\n",roleid,
				t->GetAlice()->roleid,t->GetBob()->roleid,retcode,tid);
		if (retcode==ERR_TRADE_READY_HALF || retcode==ERR_TRADE_READY)
		{
			SendResult(t,retcode);
		}
		else
		{
			//send error to roleid who want to submit
			if (retcode == ERR_TRADE_SPACE)
			{
				LOG_TRACE("gdelivery::tradesubmit: failed. no pocket space roleid=%d,tid=%d.\n",problem_roleid,tid);
				SendResult(t,retcode);
			}
			else	
				manager->Send(sid,TradeSubmit_Re(retcode,tid,roleid,roleid,localsid));
		}
	}
};

};

#endif
