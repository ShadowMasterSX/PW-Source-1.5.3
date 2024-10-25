
#ifndef __GNET_CREATEFACTIONFORTRESS_HPP
#define __GNET_CREATEFACTIONFORTRESS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailendsync.hpp"
#include "createfactionfortress_re.hpp"
#include "gmailsyncdata"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "factionfortressmanager.h"

namespace GNET
{

class CreateFactionFortress : public GNET::Protocol
{
	#include "createfactionfortress"
	void SyncGameServer( PlayerInfo& ui , const GMailSyncData& sync, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(
				ui.gameid,
				GMailEndSync(0/*tid,must be 0*/,retcode,roleid,sync)
			);
	}
	
	void SendErr( int retcode,PlayerInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				CreateFactionFortress_Re(retcode,GFactionFortressBriefInfo(),ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("createfactionfortress: receive. roleid=%d,factionid=%d,item_cost.size()=%d",
			roleid,factionid,item_cost.size());

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo )
		{
			Log::log(LOG_ERR,"gdelivery::createfactionfortress: role %d is not online", roleid);
			return;
		}

		GMailSyncData sync;
		GFactionFortressInfo info;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
			Marshal::OctetsStream os2(fortress_info);
			os2 >> info;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::createfactionfortress: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		int retcode = FactionFortressMan::GetInstance().TryCreateFortress(*this, info, *pinfo, sync);
		if(retcode != ERR_SUCCESS)
		{
			SendErr( retcode,*pinfo );
			sync.inventory.items.clear();     
			SyncGameServer( *pinfo, sync, retcode );
		}
	}
};

};

#endif
