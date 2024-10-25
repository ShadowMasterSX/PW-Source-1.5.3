
#ifndef __GNET_CHECKNEWMAIL_HPP
#define __GNET_CHECKNEWMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "postoffice.h"
#include "announcenewmail.hpp"
#include "dbgetmaillist.hrp"
#include "gamedbclient.hpp"
#include "dbdeletemail.hrp"
#include "mapuser.h"
namespace GNET
{

class CheckNewMail : public GNET::Protocol
{
	#include "checknewmail"
	bool QueryDB(PlayerInfo& ui)
	{
		DBGetMailList* rpc=(DBGetMailList*) Rpc::Call( RPC_DBGETMAILLIST,RoleId(roleid) );
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->need_send2client = false;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
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
		DEBUG_PRINT("checknewmail: receive. roleid=%d,localsid=%d\n",roleid,localsid);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			if ( !PostOffice::GetInstance().IsMailBoxValid( roleid ) )
			{
				QueryDB( *pinfo );
			}
			int remain_time=-1;
			unsigned char present_type = 0;
			if ( (remain_time=PostOffice::GetInstance().HaveNewMail( roleid,present_type ) )!=-1 )
				dsm->Send( sid,AnnounceNewMail(roleid,localsid,remain_time,present_type) );	
			DeleteExpireMail( *pinfo );
		}
	}
};

};

#endif
