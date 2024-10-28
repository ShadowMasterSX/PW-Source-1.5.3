
#ifndef __GNET_MNFACTIONPROCLAIM_RE_HPP
#define __GNET_MNFACTIONPROCLAIM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactionapplyinfo"

namespace GNET
{

class MNFactionProclaim_Re : public GNET::Protocol
{
	#include "mnfactionproclaim_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		if(retcode == ERR_SUCCESS)
		{
			CDC_MNFactionBattleMan::GetInstance()->EnableSendProclaimFlag();
		}
	}
};

};

#endif
