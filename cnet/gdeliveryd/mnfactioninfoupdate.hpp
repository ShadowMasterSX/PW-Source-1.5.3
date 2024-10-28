
#ifndef __GNET_MNFACTIONINFOUPDATE_HPP
#define __GNET_MNFACTIONINFOUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactioninfo"
#include "cdcmnfbattleman.h"

namespace GNET
{

class MNFactionInfoUpdate : public GNET::Protocol
{
	#include "mnfactioninfoupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		bool is_central = dsm->IsCentralDS();
		if(is_central)
		{
			CDS_MNFactionBattleMan::GetInstance()->UpdateCDCMNFactionInfoMap(factioninfo_list);
			CDS_MNFactionBattleMan::GetInstance()->UpdateDBMNFactionInfo(factioninfo_list);
		}
		else
		{
			//更新本地缓存
			CDC_MNFactionBattleMan::GetInstance()->UpdateMNFactionInfoMap(factioninfo_list);
			CentralDeliveryClient::GetInstance()->SendProtocol(*this);
		}
	}
};

};

#endif
