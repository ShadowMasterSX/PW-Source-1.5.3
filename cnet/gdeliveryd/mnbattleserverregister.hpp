
#ifndef __GNET_MNBATTLESERVERREGISTER_HPP
#define __GNET_MNBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "cdsmnfbattleman.h"

namespace GNET
{

class MNBattleServerRegister : public GNET::Protocol
{
	#include "mnbattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->OnRegisterServer(server_id, world_tag);
	}
};

};

#endif
