
#ifndef __GNET_MNGETFACTIONINFO_HPP
#define __GNET_MNGETFACTIONINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetFactionInfo : public GNET::Protocol
{
	#include "mngetfactioninfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid);
		bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();
		if(!is_central)
		{
			CDC_MNFactionBattleMan::GetInstance()->SendClientFactionInfo(roleid, unifid);
		}
		else
		{
			CDS_MNFactionBattleMan::GetInstance()->SendClientFactionInfo(roleid, unifid);
		}
	}
};

};

#endif
