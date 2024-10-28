
#ifndef __GNET_MNGETPLAYERLASTENTERINFO_HPP
#define __GNET_MNGETPLAYERLASTENTERINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mngetplayerlastenterinfo_re.hpp"

namespace GNET
{

class MNGetPlayerLastEnterInfo : public GNET::Protocol
{
	#include "mngetplayerlastenterinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();
		if(!is_central)
		{
			MNGetPlayerLastEnterInfo_Re re;
			re.retcode = ERR_MNF_PLAYER_NOTIN_CENTRAL;
			re.localsid = localsid;
			GDeliveryServer::GetInstance()->Send(sid, re);
		}
		else
		{
			CDS_MNFactionBattleMan::GetInstance()->SendPlayerLastEnterInfo(roleid);
		}
	}
};

};

#endif
