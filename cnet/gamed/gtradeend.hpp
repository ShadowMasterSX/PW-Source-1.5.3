
#ifndef __GNET_GTRADEEND_HPP
#define __GNET_GTRADEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void trade_end(int trade_id, int role1,int rol2,bool need_read1,bool need_read2);
namespace GNET
{

class GTradeEnd : public GNET::Protocol
{
	#include "gtradeend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		DEBUG_PRINT("gamed::Receive trade end. Tid = %d, Alice's roleid=%d(need_readDB=%d), Bob's roleid=%d(need_readDB=%d)\n",tid,roleid1,need_readDB1,roleid2,need_readDB2);
#endif
		trade_end(tid,roleid1,roleid2,need_readDB1,need_readDB2);
	}
};

};

#endif
