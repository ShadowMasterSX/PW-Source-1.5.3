
#ifndef __GNET_GTRADESTART_HPP
#define __GNET_GTRADESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gtradestart_re.hpp"
#include "gtradediscard.hpp"
#include "gproviderclient.hpp"

void trade_start(int trade_id, int role1,int role2, int localid1,int localid2);
namespace GNET
{
#ifdef _TESTCODE
class DiscardTrade : public Thread::Runnable
{
	unsigned int tid;
public:
	DiscardTrade(unsigned int _tid,int prior = 1) : Runnable(prior),tid(_tid) { }
	void Run()
	{
		GProviderClient::DispatchProtocol(0,GTradeDiscard(tid,0/*paddings*/));
		PollIO::WakeUp();
		delete this;
	}
};	
#endif
	
class GTradeStart : public GNET::Protocol
{
	#include "gtradestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		DEBUG_PRINT("gamed::receive gtradestart. tid=%d,role1=%d,role2=%d\n",tid,roleid1,roleid2);
		manager->Send(sid,GTradeStart_Re(ERR_SUCCESS,tid,roleid1,localsid1));
		manager->Send(sid,GTradeStart_Re(ERR_SUCCESS,tid,roleid2,localsid2));
		//Thread::HouseKeeper::AddTimerTask(new DiscardTrade(tid),30);
#endif
		trade_start(tid,roleid1,roleid2,localsid1,localsid2);
	}
};

};

#endif
