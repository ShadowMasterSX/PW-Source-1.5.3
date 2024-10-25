
#ifndef __GNET_DELETEMAIL_HPP
#define __GNET_DELETEMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "dbdeletemail.hrp"
#include "deletemail_re.hpp"
#include "postoffice.h"
#include "mapuser.h"
namespace GNET
{

class DeleteMail : public GNET::Protocol
{
	#include "deletemail"
	bool QueryDB( PlayerInfo& ui )
	{
		DBDeleteMailArg arg(roleid);
		arg.mailid.push_back(mail_id);
		arg.reason=MAIL_FORCE_DELETE;	
		DBDeleteMail* rpc=(DBDeleteMail*) Rpc::Call( RPC_DBDELETEMAIL,arg);
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->need_send2client=true;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	void SendErr( PlayerInfo& ui )
	{
		DEBUG_PRINT("deletemail: mail id(%d)invalid. roleid=%d,localsid=%d\n",mail_id,roleid,ui.localsid);
		GDeliveryServer::GetInstance()->Send(ui.linksid,DeleteMail_Re(ERR_MS_MAIL_INV,roleid,ui.localsid,mail_id));	   	
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("deletemail: receive. roleid=%d,mail_id=%d,localsid=%d\n",roleid,mail_id,localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo )
		{
			if ( PostOffice::GetInstance().IsMailExist(roleid,mail_id) )
				QueryDB( *pinfo );
			else
				SendErr( *pinfo );
		}
		//else discard protocol, wait PlayerHeartBeat to synchronize
	}
};

};

#endif
