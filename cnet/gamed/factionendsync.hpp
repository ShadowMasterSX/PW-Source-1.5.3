
#ifndef __GNET_FACTIONENDSYNC_HPP
#define __GNET_FACTIONENDSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "factionopsyncinfo"

#include "types.h"
#include "obj_interface.h"
#include "factionlib.h"
namespace GNET
{

class FactionEndSync : public GNET::Protocol
{
	#include "factionendsync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		printf("Receive FactionEndSync.tid=%d,roleid=%d,money=%d,sp=%d\n",tid,roleid,syncdata.player_money,syncdata.player_sp);
#endif
		FactionUnLockPlayer(tid,roleid,syncdata_t(syncdata.player_money,0));
	}
};

};

#endif
