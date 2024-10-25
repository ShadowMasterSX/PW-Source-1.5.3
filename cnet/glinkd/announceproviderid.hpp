
#ifndef __GNET_ANNOUNCEPROVIDERID_HPP
#define __GNET_ANNOUNCEPROVIDERID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"
#include "glinkserver.hpp"
#include "log.h"
namespace GNET
{

class AnnounceProviderID : public GNET::Protocol
{
	#include "announceproviderid"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		static int retry_time;
		GProviderServer* psm=GProviderServer::GetInstance();
		Thread::RWLock::WRScoped l(psm->locker_gameservermap);
		if (psm->gameservermap.find(id)!=psm->gameservermap.end())
		{
			Log::log(LOG_ERR,"Identical game server id(%d) exist. Check gameserver's \".conf\" file.",id);
			manager->Close(sid);
			return;
		}
		retry_time=0;
		psm->gameservermap[id]=sid;
		GLinkServer::GetInstance()->SetEdition(edition);
		
		this->id=psm->GetProviderServerID();
		psm->Send(sid,this);
		
	}
};

};
#endif
