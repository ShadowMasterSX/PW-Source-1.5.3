
#ifndef __GNET_MNFETCHFILTRATERESULT_RE_HPP
#define __GNET_MNFETCHFILTRATERESULT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "cdcmnfbattleman.h"
namespace GNET
{

class MNFetchFiltrateResult_Re : public GNET::Protocol
{
	#include "mnfetchfiltrateresult_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDC_MNFactionBattleMan::GetInstance()->OnGetCDSFiltrateResult(chosen_list);
	}
};

};

#endif
