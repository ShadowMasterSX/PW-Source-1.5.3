
#ifndef __GNET_MNDOMAINBATTLEEND_HPP
#define __GNET_MNDOMAINBATTLEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleEnd : public GNET::Protocol
{
	#include "mndomainbattleend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->OnDomainBattleEnd(domain_id, winner_fid);
	}
};

};

#endif
