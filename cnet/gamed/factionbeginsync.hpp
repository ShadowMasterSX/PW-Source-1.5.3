
#ifndef __GNET_FACTIONBEGINSYNC_HPP
#define __GNET_FACTIONBEGINSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE
#	include "factionbeginsync_re.hpp"
#endif
namespace GNET
{
void FactionLockPlayer(unsigned int tid,int roleid);
class FactionBeginSync : public GNET::Protocol
{
	#include "factionbeginsync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		printf("receive factoinbeginsync from factionserver.\n");
		FactionOPSyncInfo fsi(50000/*money*/,800/*sp*/);
		manager->Send(sid,FactionBeginSync_Re(ERR_SUCCESS,tid,roleid,fsi));
#endif
		FactionLockPlayer(tid,roleid);
	}
};

};

#endif
