
#ifndef __GNET_MNGETDOMAINDATA_HPP
#define __GNET_MNGETDOMAINDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"

namespace GNET
{

class MNGetDomainData : public GNET::Protocol
{
	#include "mngetdomaindata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();
		if(!is_central)
		{
			CDC_MNFactionBattleMan::GetInstance()->SendClientDomainData(roleid);
		}
		else
		{
			CDS_MNFactionBattleMan::GetInstance()->SendClientDomainData(roleid);
		}
	}
};

};

#endif
