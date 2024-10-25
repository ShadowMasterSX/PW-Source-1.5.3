
#ifndef __GNET_KEVOTING_HPP
#define __GNET_KEVOTING_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"
#include "gmailendsync.hpp"
#include "kevoting_re.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "kingelection.h"

namespace GNET
{

class KEVoting : public GNET::Protocol
{
	#include "kevoting"
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
				KEVoting_Re(retcode,ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("kevoting: receive. roleid=%d,item_id=%d,item_pos=%d,item_num=%d,candidate_roleid=%d\n", roleid,item_id,item_pos,item_num,candidate_roleid);

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo )
		{
			Log::log(LOG_ERR,"kevoting: role %d is not online", roleid);
			return;
		}

		GMailSyncData sync;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"kevoting: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		int retcode = KingElection::GetInstance().TryVote(roleid, item_id, item_pos, item_num, candidate_roleid, *pinfo, sync);
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
