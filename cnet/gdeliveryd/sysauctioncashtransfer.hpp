
#ifndef __GNET_SYSAUCTIONCASHTRANSFER_HPP
#define __GNET_SYSAUCTIONCASHTRANSFER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "sysauctionmanager.h"
#include "sysauctioncashtransfer_re.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

namespace GNET
{

class SysAuctionCashTransfer : public GNET::Protocol
{
	#include "sysauctioncashtransfer"
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
				SysAuctionCashTransfer_Re(retcode,0,ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sysauctioncashtransfer: receive. roleid=%d,localsid=%d,withdraw=%d,cash=%u", roleid, localsid, withdraw, cash);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo ) return;

		GMailSyncData sync;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::sysauctioncashtransfer: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		int retcode = SysAuctionManager::GetInstance().TryCashTransfer(withdraw, cash, *pinfo, sync);
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
