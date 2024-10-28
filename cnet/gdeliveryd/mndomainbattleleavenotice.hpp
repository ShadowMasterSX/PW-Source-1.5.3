
#ifndef __GNET_MNDOMAINBATTLELEAVENOTICE_HPP
#define __GNET_MNDOMAINBATTLELEAVENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleLeaveNotice : public GNET::Protocol
{
	#include "mndomainbattleleavenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->OnPlayerBattleLeave(roleid, unifid, domain_id);
	}
};

};

#endif
