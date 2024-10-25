
#ifndef __GNET_STOCKCANCEL_HPP
#define __GNET_STOCKCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class StockCancel : public GNET::Protocol
{
	#include "stockcancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("StockCancel, roleid=%d,tid=%d", roleid, tid);
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(pinfo)
			StockExchange::Instance()->PreCancel(pinfo->userid, tid, price);
	}
};

};

#endif
