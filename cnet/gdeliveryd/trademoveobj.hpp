
#ifndef __GNET_TRADEMOVEOBJ_HPP
#define __GNET_TRADEMOVEOBJ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"

#include "gdeliveryserver.hpp"
#include "trade.h"
#include "trademoveobj_re.hpp"
#include "mapuser.h"
namespace GNET
{

class TradeMoveObj : public GNET::Protocol
{
	#include "trademoveobj"
	void SendResult(GNET::Transaction* t,int retcode)
	{
		//send result to both traders
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);
		if (NULL!=pinfo)
			dsm->Send(pinfo->linksid,TradeMoveObj_Re(retcode,tid,roleid,localsid,goods.pos,goods.count,dst_pos));
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::Transaction* t=GNET::Transaction::GetTransaction(tid);
		if (t == NULL) return;
		Log::formatlog("trade_debug","trademovegoods: roleid=%d,good is (id=%d,pos=%d,count=%d),dst_pos=%d,tid=%d\n",
				roleid,goods.id,goods.pos,goods.count,dst_pos,tid);
		int retcode;
		if ((retcode=t->MoveObject(roleid, goods,dst_pos))==ERR_SUCCESS)
		{
			SendResult(t,retcode);
		}
		else
		{
			//send error to roleid who want to addgoods
			manager->Send(sid,TradeMoveObj_Re(retcode,tid,roleid,localsid,goods.pos,goods.count,dst_pos));
		}

	}
};

};

#endif
