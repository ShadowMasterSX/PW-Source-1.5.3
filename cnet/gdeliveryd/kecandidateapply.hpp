
#ifndef __GNET_KECANDIDATEAPPLY_HPP
#define __GNET_KECANDIDATEAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"
#include "gmailendsync.hpp"
#include "kecandidateapply_re.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "kingelection.h"

namespace GNET
{

class KECandidateApply : public GNET::Protocol
{
	#include "kecandidateapply"
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
				KECandidateApply_Re(retcode,ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("kecandidateapply: receive. roleid=%d,item_id=%d,item_num=%d\n", roleid,item_id,item_num);

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo )
		{
			Log::log(LOG_ERR,"kecandidateapply: role %d is not online", roleid);
			return;
		}

		GMailSyncData sync;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"kecandidateapply: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		int retcode = KingElection::GetInstance().TryCandidateApply(roleid, item_id, item_num, *pinfo, sync);
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
