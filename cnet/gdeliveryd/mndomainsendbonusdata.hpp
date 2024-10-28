
#ifndef __GNET_MNDOMAINSENDBONUSDATA_HPP
#define __GNET_MNDOMAINSENDBONUSDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mndomainbonus"

namespace GNET
{

class MNDomainSendBonusData : public GNET::Protocol
{
	#include "mndomainsendbonusdata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDC_MNFactionBattleMan::GetInstance()->OnRecvBonusData(bonus);
	}
};

};

#endif
