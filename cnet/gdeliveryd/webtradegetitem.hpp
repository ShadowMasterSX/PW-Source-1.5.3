
#ifndef __GNET_WEBTRADEGETITEM_HPP
#define __GNET_WEBTRADEGETITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradegetitem_re.hpp"

namespace GNET
{

class WebTradeGetItem : public GNET::Protocol
{
	#include "webtradegetitem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtradegetitem: receive. roleid=%d,localsid=%d,snset.size=%d\n",
				roleid,localsid,sns.size());
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid );
		if ( NULL!=pinfo )
		{
			WebTradeGetItem_Re re;
			re.localsid = localsid; 
			GWebTradeDetail detail;
			for ( size_t i=0;i<sns.size() && i<WEBTRADE_PAGE_SIZE;++i )
			{
				if ( WebTradeMarket::GetInstance().GetWebTrade(sns[i],detail) )
				{
					if(detail.info.posttype == 2)
					{
						re.sns.push_back( sns[i] );
						re.items.push_back( detail.item );
					}
					else if(detail.info.posttype == 4)
					{
						//未与平台同步完毕的寄售角色由于数据量过大，不再发给客户端
						re.sns.push_back( sns[i] );
						if(detail.rolebrief.size() > 256)
							re.rolebriefs.push_back( Octets() );
						else
							re.rolebriefs.push_back( detail.rolebrief );
					}
				}
			}
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,re );
		}
	}
};

};

#endif
