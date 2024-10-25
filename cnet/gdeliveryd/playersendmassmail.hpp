
#ifndef __GNET_PLAYERSENDMASSMAIL_HPP
#define __GNET_PLAYERSENDMASSMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "postoffice.h"
#include "dbsendmassmail.hrp"
#include "playersendmail_re.hpp"
#include "gmailendsync.hpp"
#include "gproviderserver.hpp"
#include "mapuser.h"

namespace GNET
{
#define MAX_MASS_COUNT 300
class PlayerSendMassMail : public GNET::Protocol
{
	#include "playersendmassmail"

	int MakeMail(GMail& mail,const Octets& rolename/*sender name*/)
	{
		mail.header.id        = 0; // need fill by gamedbd
		mail.header.sender    = roleid;
		mail.header.sndr_type = _MST_PLAYER;
		mail.header.receiver  = 0;
		mail.header.title     = title;
		mail.header.send_time = time(NULL);
		mail.header.attribute = (1<<_MA_UNREAD);
		mail.header.sender_name = rolename;

		mail.context = context;

		return ERR_SUCCESS;
	}
	void QueryDB( PlayerInfo& ui, GMailSyncData& data)
	{
		int ret;
		DBSendMassMailArg arg(mass_type);
		if ( (ret=MakeMail(arg.mail,sender_name.size() ? sender_name : ui.name))!=ERR_SUCCESS )
		{
			SendErr( ret,ui,data );
			return;
		}
		arg.mass_type = mass_type;
		arg.syncdata = data;
		arg.cost_money = cost_money;
		arg.cost_obj_id = cost_obj_id;
		arg.cost_obj_num = cost_obj_num;
		arg.cost_obj_pos = cost_obj_pos;
		arg.receiver_list.swap(receiver_list);
		DBSendMassMail* rpc=(DBSendMassMail*) Rpc::Call( RPC_DBSENDMASSMAIL, arg);
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->save_gsid=ui.gameid;
		GameDBClient::GetInstance()->SendProtocol( rpc );
	}

	void SendErr( int errcode, PlayerInfo& ui, GMailSyncData& data )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				PlayerSendMail_Re(errcode,roleid,ui.localsid,0,cost_obj_num,cost_obj_pos,cost_money)
			);
		data.inventory.items.clear();
		GProviderServer::GetInstance()->DispatchProtocol(ui.gameid, GMailEndSync(0,errcode, roleid, data));	
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("playersendmassmail: receive. roleid=%d,localsid=%d,masstype=%d,cost_obj(id:%d,pos:%d,num:%d),cost_money=%d\n", 
			roleid,localsid,mass_type,cost_obj_id,cost_obj_pos,cost_obj_num,cost_money);
		
		int retcode = 0;
		GMailSyncData data;
		try{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::playersendmassmail: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}
		
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if(dsm->IsCentralDS())
		{
			retcode = ERR_MS_AGAIN;
		} 
		else if(receiver_list.empty() || receiver_list.size() > MAX_MASS_COUNT)
		{
			retcode = ERR_MS_SEND_SELF;
		}

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo)
		{
			if(retcode!=ERR_SUCCESS)
				SendErr( retcode,*pinfo,data);
			else
				QueryDB( *pinfo,data);
		}
	}
};

};

#endif
