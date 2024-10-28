
#ifndef __GNET_MNDOMAINFETCHBONUSDATA_HPP
#define __GNET_MNDOMAINFETCHBONUSDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "cdsmnfbattleman.h"

namespace GNET
{

class MNDomainFetchBonusData : public GNET::Protocol
{
	#include "mndomainfetchbonusdata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->SendDomainBonusData(zoneid);
	}
};

};

#endif
