
#ifndef __GNET_GETMAILATTACHOBJ_HPP
#define __GNET_GETMAILATTACHOBJ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"
#include "gmailsyncdata"
#include "getmailattachobj_re.hpp"
#include "gmailendsync.hpp"
#include "gproviderserver.hpp"
#include "mapuser.h"
namespace GNET
{

class GetMailAttachObj : public GNET::Protocol
{
	#include "getmailattachobj"
	bool QueryDB( PlayerInfo& ui , GMailSyncData& data)
	{
		DBGetMailAttach* rpc=(DBGetMailAttach*) Rpc::Call(
				RPC_DBGETMAILATTACH,
				DBGetMailAttachArg(
					GMailID(roleid,mail_id),
					obj_type,
					data
				)
			);
		rpc->save_gsid=ui.gameid;
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	bool SendErr( PlayerInfo& ui )
	{
		Log::log(LOG_WARNING,"getmailattachobj: mail id(%d) invalid. roleid=%d,localsid=%d\n",
				mail_id,roleid,localsid);
		return GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				GetMailAttachObj_Re(ERR_MS_MAIL_INV,roleid,localsid,mail_id,obj_type)
			);	
	}
	bool SyncGameServer( PlayerInfo& ui , GMailSyncData& data, int retcode)
	{
		return GProviderServer::GetInstance()->DispatchProtocol( ui.gameid, GMailEndSync(0,retcode, roleid, data));	
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("getmailattachobj: receive. roleid=%d,mail_id=%d,obj_type=%d\n",roleid,mail_id,obj_type);
		GMailSyncData data;
		try{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::playersendmail: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo)
		{
			if ( PostOffice::GetInstance().IsMailExist(roleid,mail_id) )
				QueryDB( *pinfo, data );
			else
			{
				SendErr( *pinfo );
				data.inventory.items.clear();
				SyncGameServer( *pinfo , data, ERR_MS_MAIL_INV);
			}
		}
	}
};

};

#endif
