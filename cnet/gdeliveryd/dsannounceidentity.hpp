
#ifndef __GNET_DSANNOUNCEIDENTITY_HPP
#define __GNET_DSANNOUNCEIDENTITY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "centraldeliveryserver.hpp"
#include "centraldeliveryclient.hpp"

namespace GNET
{

class DSAnnounceIdentity : public GNET::Protocol
{
	#include "dsannounceidentity"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("CrossRelated Recv DSAnnounceIdentity zoneid %d version %d edition.size %d", zoneid, version, edition.size());
		
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if(dsm->IsCentralDS()) {
			if(!CentralDeliveryServer::GetInstance()->IsAcceptedZone(zoneid)) {
				Log::log(LOG_ERR, "CrossRelated DSAnnounceIdentity Zoneid %d is not accepted", zoneid);
				manager->Close(sid);
				return;
			}
			
			if(CentralDeliveryServer::GetInstance()->IsConnect(zoneid)) {
				Log::log(LOG_ERR, "CrossRelated DSAnnounceIdentity Identical Zoneid %d", zoneid);
				manager->Close(sid);
				return;
			}
			
			if(version != dsm->GetVersion() || edition != dsm->GetEdition()) {
				Log::log(LOG_ERR, "CrossRelated CentralDeliveryClient zoneid(%d) version-edition.size %d-%d does not match Server(local) %d-%d, disconnect connection", 
					zoneid, version, edition.size(), dsm->GetVersion(), dsm->GetEdition().size());
				manager->Close(sid);
				return;
			}
			
			LOG_TRACE("CrossRelated Accept CentralDSClient zoneid(%d) and send back", zoneid);
			
			CentralDeliveryServer::GetInstance()->InsertZoneId(zoneid, sid);

			this->zoneid = GDeliveryServer::GetInstance()->GetZoneid();
			manager->Send(sid, this);
		} else {
			CentralDeliveryClient::GetInstance()->SetServerZoneid(zoneid);
		}
	}
};

};

#endif
