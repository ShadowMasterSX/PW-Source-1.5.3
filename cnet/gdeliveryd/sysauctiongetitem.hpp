
#ifndef __GNET_SYSAUCTIONGETITEM_HPP
#define __GNET_SYSAUCTIONGETITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "sysauctionmanager.h"
#include "sysauctiongetitem_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class SysAuctionGetItem : public GNET::Protocol
{
	#include "sysauctiongetitem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sysauctiongetitem: receive. roleid=%d,localsid=%d,ids.size()=%d", roleid, localsid, ids.size());
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			SysAuctionGetItem_Re re;
			re.localsid = localsid;
			GSysAuctionDetail detail;
			for(size_t i=0; i<ids.size(); i++)
			{
				if(SysAuctionManager::GetInstance().GetSysAuction(ids[i],detail))
				{
					re.ids.push_back(ids[i]);
					re.items.push_back(detail.item);
				}
			}
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,re );
		}
	}
};

};

#endif
