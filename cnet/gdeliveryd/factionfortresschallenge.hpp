
#ifndef __GNET_FACTIONFORTRESSCHALLENGE_HPP
#define __GNET_FACTIONFORTRESSCHALLENGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailendsync.hpp"
#include "factionfortresschallenge_re.hpp"
#include "factionfortressmanager.h"

namespace GNET
{

class FactionFortressChallenge : public GNET::Protocol
{
	#include "factionfortresschallenge"
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
				FactionFortressChallenge_Re(retcode,ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("factionfortresschallenge: receive. roleid=%d, factionid=%d, target_factionid=%d",roleid,factionid,target_factionid);

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo )
		{
			Log::log(LOG_ERR,"gdelivery::factionfortresschallenge: role %d is not online", roleid);
			return;
		}
		
		GMailSyncData sync;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::factionfortresschallenge: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		int retcode = FactionFortressMan::GetInstance().TryChallenge(*this, *pinfo, sync);
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
