
#ifndef __GNET_TRADEMOVEOBJ_RE_HPP
#define __GNET_TRADEMOVEOBJ_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"

#include "glinkclient.hpp"
#include "tradechoice.h"
namespace GNET
{

class TradeMoveObj_Re : public GNET::Protocol
{
	#include "trademoveobj_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch (retcode)
		{
			case ERR_TRADE_INVALID_TRADER:
				DEBUG_PRINT("Move goods failed. You are invalid trader.\n");
				break;
			case ERR_TRADE_MOVE_FAIL:
				DEBUG_PRINT("Move goods failed. You are not allowed to move now, or you can not move objects in exchange.\n");
				TradeChoice(tid,roleid,manager,sid);
				break;
			case ERR_SUCCESS:
				DEBUG_PRINT("Move goods(pos=%d,count=%d) to pos=%d successfully.\n",src_pos,count,dst_pos);	
				TradeChoice(tid,roleid,manager,sid);
				break;
		}
	}
};

};

#endif
