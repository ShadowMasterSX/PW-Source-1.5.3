
#ifndef __GNET_GETMAIL_HPP
#define __GNET_GETMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "dbgetmail.hrp"
#include "getmail_re.hpp"
#include "postoffice.h"
#include "mapuser.h"
namespace GNET
{

class GetMail : public GNET::Protocol
{
	#include "getmail"
	// this protocolo is send from gameserver
	bool QueryDB( PlayerInfo& ui )
	{
		DBGetMail* rpc=(DBGetMail*)Rpc::Call( RPC_DBGETMAIL,GMailID(roleid,mail_id) );
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	void SendErr( PlayerInfo& ui )
	{
		DEBUG_PRINT("getmail: mail id invalid. roleid=%d,mail_id=%d,localsid=%d\n",roleid,mail_id,localsid);
		GDeliveryServer::GetInstance()->Send( ui.linksid, GetMail_Re(ERR_MS_MAIL_INV,roleid,ui.localsid));
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("getmail: receive. roleid=%d,mail_id=%d,localsid=%d\n",roleid,mail_id,localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo )
		{
			if ( PostOffice::GetInstance().IsMailExist(roleid,mail_id) )
				QueryDB( *pinfo );
			else
				SendErr( *pinfo );
		}
	}
};

};

#endif
