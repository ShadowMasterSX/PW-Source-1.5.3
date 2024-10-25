
#ifndef __GNET_PRESERVEMAIL_HPP
#define __GNET_PRESERVEMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "postoffice.h"
#include "preservemail_re.hpp"
#include "dbsetmailattr.hrp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "mapuser.h"
namespace GNET
{

class PreserveMail : public GNET::Protocol
{
	#include "preservemail"
	bool QueryDB( PlayerInfo& ui )
	{
		DBSetMailAttr* rpc=(DBSetMailAttr*) Rpc::Call( 
				RPC_DBSETMAILATTR,
				DBSetMailAttrArg( 
				GMailID(roleid,mail_id),
				_MA_PRESERVE,
				blPreserve
				) 
			);
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	void SendErr( PlayerInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				PreserveMail_Re(ERR_MS_MAIL_INV,roleid,ui.localsid,mail_id,blPreserve)
			);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("preservemail: receive. roleid=%d,mail_id=%d,blPreserve=%d\n",roleid,mail_id,blPreserve);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo )
		{
			if ( PostOffice::GetInstance().IsMailExist( roleid,mail_id ) )
			{
				QueryDB( *pinfo );
			}
			else
				SendErr( *pinfo ); 	
		}
	}
};

};

#endif
