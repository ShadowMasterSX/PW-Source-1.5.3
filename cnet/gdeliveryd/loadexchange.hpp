
#ifndef __GNET_LOADEXCHANGE_HPP
#define __GNET_LOADEXCHANGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "centraldeliveryclient.hpp"
#include "mapuser.h"
#include "gdeliveryserver.hpp"

namespace GNET
{

class LoadExchange : public GNET::Protocol
{
	#include "loadexchange"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("loadexchange zoneid %d version %d edition.size %d srv_limit %d srv_count %d", 
				zoneid, version, edition.size(), server_limit, server_count);
		
		if(GDeliveryServer::GetInstance()->IsCentralDS()) {
			CentralDeliveryServer* cds = CentralDeliveryServer::GetInstance();
			cds->SetLoad(zoneid, server_limit, server_count);
		} else {
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			
			if(edition == dsm->GetEdition() && version == dsm->GetVersion()) {
				CentralDeliveryClient::GetInstance()->SetLoad(server_limit, server_count);
				this->zoneid = dsm->GetZoneid();
				this->version = dsm->GetVersion();
				this->edition = dsm->GetEdition();
				//普通服务器的 server_limit 适当放宽 防止跳回原服时出现服务器人数满的情况
				this->server_limit = UserContainer::GetInstance().GetPlayerLimit() + 2000;
				this->server_count = UserContainer::GetInstance().Size();
				manager->Send(sid, this);
			} else {
				Log::log(LOG_ERR, "LoadEXchange CentralDeliveryServer version-edition.size %d-%d does not match local %d-%d, disconnect connection",
						version, edition.size(), dsm->GetVersion(), dsm->GetEdition().size());
				manager->Close(sid);
			}
		}
	}
};

};

#endif
