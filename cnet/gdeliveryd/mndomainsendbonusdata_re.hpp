
#ifndef __GNET_MNDOMAINSENDBONUSDATA_RE_HPP
#define __GNET_MNDOMAINSENDBONUSDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainSendBonusData_Re : public GNET::Protocol
{
	#include "mndomainsendbonusdata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%dunifid=%lld\n", __FILE__, __FUNCTION__, __LINE__, unifid);
		CDS_MNFactionBattleMan::GetInstance()->OnRecvSendBonusDataResult(unifid);
	}
};

};

#endif
