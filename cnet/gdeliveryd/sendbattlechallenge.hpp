
#ifndef __GNET_SENDBATTLECHALLENGE_HPP
#define __GNET_SENDBATTLECHALLENGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "battlechallenge_re.hpp"
#include "groleinventory"
#include "gmailendsync.hpp"
#include "mapuser.h"
#include "gdeliveryserver.hpp"
#include "postoffice.h"
#include "gproviderserver.hpp"

namespace GNET
{

class SendBattleChallenge : public GNET::Protocol
{
	#include "sendbattlechallenge"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("sendbattlechallenge");
		unsigned int linkid, gsid;
		int retcode = 0;
		if( PostOffice::GetInstance().GetMailBoxSize(roleid) >= SYS_MAIL_LIMIT)
			retcode = ERR_AS_MAILBOXFULL;	
		BattleChallenge_Re res(0, roleid, id, 0, maxbonus, 0, 0, 0);
		if(!BattleManager::GetInstance()->FindSid(roleid, res.localsid, linkid, gsid))
			retcode = ERR_BS_OUTOFSERVICE;	
		if(!retcode)
			retcode = BattleManager::GetInstance()->Challenge(*this, res);
		if(retcode!=ERR_SUCCESS)
		{
			res.retcode = retcode;
			syncdata.inventory.items.clear();
			GDeliveryServer::GetInstance()->Send(linkid, res);
			GProviderServer::GetInstance()->DispatchProtocol(gsid, GMailEndSync(0,retcode,roleid,syncdata));
		}
	}
};

};

#endif
