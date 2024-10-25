
#ifndef __GNET_GETMAILLIST_HPP
#define __GNET_GETMAILLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "postoffice.h"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "dbgetmaillist.hrp"
#include "getmaillist_re.hpp"
#include "dbdeletemail.hrp"
#include "mapuser.h"
namespace GNET
{

class GetMailList : public GNET::Protocol
{
	#include "getmaillist"
	//this protocol is send from gameserver
	bool QueryDB(PlayerInfo& ui)
	{
		DBGetMailList* rpc=(DBGetMailList*) Rpc::Call( RPC_DBGETMAILLIST,RoleId(roleid) );
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->need_send2client = true;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	void SendResult(PlayerInfo& ui)
	{
		GetMailList_Re gml_re(ERR_SUCCESS,roleid,ui.localsid);
		PostOffice::GetInstance().GetMailList(roleid,gml_re.maillist);
		GDeliveryServer::GetInstance()->Send( ui.linksid,gml_re );
	}
	void DeleteExpireMail( PlayerInfo& ui)
	{
		GMailIDVector maillist;
		if ( PostOffice::GetInstance().CheckExpireMail(roleid,maillist) &&
			 maillist.size() )
		{
			DBDeleteMailArg arg(roleid);
			for ( size_t i=0;i<maillist.size();++i ) {
				arg.mailid.push_back( maillist[i].mail_id );
			}
			arg.reason=0; //not FORCE_DELETE
			DBDeleteMail* rpc=(DBDeleteMail*) Rpc::Call( RPC_DBDELETEMAIL,arg );
			rpc->save_linksid=ui.linksid;
			rpc->save_localsid=ui.localsid;
			rpc->need_send2client=false;
			GameDBClient::GetInstance()->SendProtocol( rpc );
		}
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("getmaillist: receive. roleid=%d,localsid=%d\n",roleid,localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo)
		{
			if ( PostOffice::GetInstance().IsMailBoxValid(roleid) )
			{
				SendResult( *pinfo );
				DeleteExpireMail( *pinfo );
			}
			else
				QueryDB( *pinfo );
		}
		//else 
			//can not find the role
			//discard protocol, wait playerheartbeat to synchronize.
	}
};

};

#endif
