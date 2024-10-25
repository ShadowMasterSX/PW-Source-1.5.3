
#ifndef __GNET_PLAYERSENDMAIL_HPP
#define __GNET_PLAYERSENDMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "postoffice.h"
#include "dbsendmail.hrp"
#include "playersendmail_re.hpp"
#include "gmailendsync.hpp"
#include "gproviderserver.hpp"
#include "mapuser.h"
namespace GNET
{

class PlayerSendMail : public GNET::Protocol
{
	#include "playersendmail"
	bool HasAttachObj()   { return attach_obj_num!=0; }
	bool HasAttachMoney() { return attach_money!=0;   }
	int MakeMail(GMail& mail,const Octets& rolename/*sender name*/)
	{
		mail.header.id        = 0; // need fill by gamedbd
		mail.header.sender    = roleid;
		mail.header.sndr_type = _MST_PLAYER;
		mail.header.receiver  = receiver;
		mail.header.title     = title;
		mail.header.send_time = time(NULL);
		mail.header.attribute = (1<<_MA_UNREAD);
		mail.header.sender_name = rolename;
		if ( HasAttachObj() )   mail.header.attribute |= 1<<_MA_ATTACH_OBJ;
		if ( HasAttachMoney() ) mail.header.attribute |= 1<<_MA_ATTACH_MONEY;

		mail.context = context;
		if ( HasAttachObj() )
		{
			mail.attach_obj.pos=attach_obj_pos;
			mail.attach_obj.count=attach_obj_num;
			mail.attach_obj.id=attach_obj_id;
		}
		if ( HasAttachMoney() )
		{
			mail.attach_money=attach_money;
		}
		return ERR_SUCCESS;
	}
	void QueryDB( PlayerInfo& ui, GMailSyncData& data )
	{
		int ret;
		DBSendMailArg arg;
		if ( (ret=MakeMail(arg.mail,ui.name))!=ERR_SUCCESS )
		{
			SendErr( ret,ui,data );
			return;
		}
		arg.syncdata = data;
		DBSendMail* rpc=(DBSendMail*) Rpc::Call( RPC_DBSENDMAIL, arg);
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->save_gsid=ui.gameid;
		GameDBClient::GetInstance()->SendProtocol( rpc );
	}

	void SendErr( int errcode, PlayerInfo& ui, GMailSyncData& data )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				PlayerSendMail_Re(errcode,roleid,ui.localsid,receiver,attach_obj_num,attach_obj_pos,attach_money)
			);
		data.inventory.items.clear();
		GProviderServer::GetInstance()->DispatchProtocol(ui.gameid, GMailEndSync(0,errcode, roleid, data));	
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("playersendmail: receive. roleid=%d,localsid=%d,receiver=%d,attach_obj(id:%d,pos:%d,num:%d),attach_money=%d\n", 
			roleid,localsid,receiver,attach_obj_id,attach_obj_pos,attach_obj_num,attach_money);
		
		int retcode = 0;
		GMailSyncData data;
		try{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::playersendmail: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}
		
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if(dsm->IsCentralDS())
		{
			retcode = ERR_MS_AGAIN;
		} 
		else if(PostOffice::GetInstance().GetMailBoxSize(receiver) >= PLAYER_MAIL_LIMIT)
		{
			retcode = ERR_MS_BOXFULL;
		}

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo)
		{
			if(retcode!=ERR_SUCCESS)
				SendErr( retcode,*pinfo,data);
			else if ( receiver==roleid )
				SendErr( ERR_MS_SEND_SELF,*pinfo,data );
			else
				QueryDB( *pinfo,data );
		}
	}
};

};

#endif
