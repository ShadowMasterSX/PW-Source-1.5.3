
#ifndef __GNET_MNDOMAINBATTLEENTER_HPP
#define __GNET_MNDOMAINBATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleEnter : public GNET::Protocol
{
	#include "mndomainbattleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->OnPlayerBattleEnter(roleid, unifid, domain_id);
	}
};

};

#endif
