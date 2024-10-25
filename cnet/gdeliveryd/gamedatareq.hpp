
#ifndef __GNET_GAMEDATAREQ_HPP
#define __GNET_GAMEDATAREQ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "snsclient.hpp"
#include "snsmanager.h"


namespace GNET
{

class GameDataReq : public GNET::Protocol
{
	#include "gamedatareq"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(manager == SNSClient::GetInstance())
		{
			switch(dtype)
			{
				case SNS_DTYPE_ROLEBRIEF:
					break;
					
				case SNS_DTYPE_ROLESKILLS:
				case SNS_DTYPE_ROLEEQUIPMENT:
				case SNS_DTYPE_ROLEPETCORRAL:
					SNSManager::GetInstance()->FetchRoleData((int)id, dtype);	
					break;
				case SNS_DTYPE_FACTIONEXT:
					SNSManager::GetInstance()->FetchFactionExt((int)id,dtype);
					break;
				case SNS_DTYPE_CITY:
					SNSManager::GetInstance()->FetchCityInfo((int)id,dtype);
					break;
				default:
					break;
			}
		}
	}
};

};

#endif
