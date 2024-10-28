
#ifndef __GNET_MNDOMAINBATTLEENTERSUCCESSNOTICE_HPP
#define __GNET_MNDOMAINBATTLEENTERSUCCESSNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "cdsmnfbattleman.h"
namespace GNET
{

class MNDomainBattleEnterSuccessNotice : public GNET::Protocol
{
	#include "mndomainbattleentersuccessnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->OnPlayerBattleEnterSuccess(roleid, unifid, domain_id);
	}
};

};

#endif
