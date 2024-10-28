
#ifndef __GNET_MNFETCHFILTRATERESULT_HPP
#define __GNET_MNFETCHFILTRATERESULT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "cdsmnfbattleman.h"
namespace GNET
{

class MNFetchFiltrateResult : public GNET::Protocol
{
	#include "mnfetchfiltrateresult"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDS_MNFactionBattleMan::GetInstance()->SendFiltrateResult(zoneid);
	}
};

};

#endif
