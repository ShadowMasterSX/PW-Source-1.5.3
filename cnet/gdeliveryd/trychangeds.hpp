
#ifndef __GNET_TRYCHANGEDS_HPP
#define __GNET_TRYCHANGEDS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "centraldeliveryserver.hpp"
#include "centraldeliveryclient.hpp"

namespace GNET
{

class TryChangeDS : public GNET::Protocol
{
	#include "trychangeds"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("CrossRelated TryChangeDS:roleid=%d, flag=%d, cross_type=%d, unifid=%lld", roleid, flag, cross_type, unifid);
		
		PlayerInfo* pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(pinfo == NULL /*|| BlockedRole::GetInstance()->IsRoleBlocked(roleid)*/) return;
		
		int ret = ERR_SUCCESS;
		if(flag == DS_TO_CENTRALDS) { //原服->跨服
			if(ret == ERR_SUCCESS && GDeliveryServer::GetInstance()->IsCentralDS()) ret = -1;	
			if(ret == ERR_SUCCESS && (!CrossGuardClient::GetInstance()->CanCross() && !pinfo->IsGM())) ret = ERR_CDS_COMMUNICATION;
			if(ret == ERR_SUCCESS && !CentralDeliveryClient::GetInstance()->IsConnect()) ret = ERR_CDS_COMMUNICATION;

			if(ret == ERR_SUCCESS) // 根据活动判断
			{
				switch(cross_type)
				{
					case CT_MNFACTION_BATTLE:
						ret = CDC_MNFactionBattleMan::GetInstance()->CheckMNFactionPlayerCross(unifid);
						break;
				}
			}
			//诛仙在此有选线的逻辑，完美国际没有线的概念，不需要，但依然要检查跨服服务器是否超载
			if(ret == ERR_SUCCESS) {
				ret = CentralDeliveryClient::GetInstance()->CheckLimit();
			}
		} else if(flag == CENTRALDS_TO_DS) { //跨服->原服
			if(ret == ERR_SUCCESS && !GDeliveryServer::GetInstance()->IsCentralDS()) ret = -1;

			CrossInfoData* pCrsRole = pinfo->user->GetCrossInfo(roleid);
			if(ret == ERR_SUCCESS && pCrsRole == NULL) ret = -2;

			if(ret == ERR_SUCCESS && !CentralDeliveryServer::GetInstance()->IsConnect(pCrsRole->src_zoneid)) ret = ERR_COMMUNICATION;
			
			//诛仙在此有选线的逻辑，完美国际没有线的概念，不需要，也不需要检查原服是否超载，因为目前原服必然没有超载
		} else {
			ret = -1;
		}
		
		if (ret != ERR_SUCCESS) {
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, ChangeDS_Re(ret, pinfo->localsid));
		} else {
			GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, PlayerChangeDS(roleid, flag));
		}
	}
};

};

#endif
