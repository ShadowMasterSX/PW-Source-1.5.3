
#ifndef __GNET_PLAYERGIVEPRESENT_HPP
#define __GNET_PLAYERGIVEPRESENT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"
#include "postoffice.h"
#include "dbplayergivepresent.hrp"
#include "playergivepresent_re.hpp"

namespace GNET
{

class PlayerGivePresent : public GNET::Protocol
{
	#include "playergivepresent"
	
	void SyncGameServer(const PlayerInfo & pinfo, const GMailSyncData& sync, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(
				pinfo.gameid,
				GMailEndSync(0/*tid,must be 0*/,retcode,roleid,sync)
			);
	}

	void SendErr( PlayerInfo& ui , int retcode)
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				PlayerGivePresent_Re(ui.localsid, target_roleid, cash_cost, retcode)
			);
	}
	
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("PlayerGivePresent roleid=%d,targetroleid=%d,cash_cost=%d,log_price1=%d,log_price2=%d,mail_id=%d",
			   	roleid,target_roleid,cash_cost,log_price1,log_price2,mail_id);
		GMailSyncData sync;
		try{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}
		catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"PlayerGivePresent unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo ) return;

		int retcode = 0;
		GDeliveryServer * dsm=GDeliveryServer::GetInstance(); 
		if (dsm->IsCentralDS())	retcode = ERR_GP_OUTOFSERVICE;

		if ( 0 == retcode && mail_id >= 0 )
		{
			GMailHeader mail;
			if (PostOffice::GetInstance().GetMail(roleid, mail_id, mail))
			{
				if (mail.sndr_type != _MST_PLAYERPRESENT || mail.sender != PLAYERPRESENT_ASK)
					retcode = ERR_GP_MAIL_ERR;
			}
			else
				retcode = ERR_GP_MAIL_ERR;
		}
		
		//判断赠送对象的邮箱是否已经满了，由于可能存在赠品，所以至少要2空位
		if( 0 == retcode && PostOffice::GetInstance().GetMailBoxSize(target_roleid)
			   	> SYS_MAIL_LIMIT-2 )
			retcode = ERR_GP_GIVE_TARGET_MAILBOX_FULL;

		if (retcode)
		{
			DEBUG_PRINT("PlayerGivePresent err, retcode=%d",retcode);
			// 解锁玩家交易状态
			sync.inventory.items.clear();
			SyncGameServer(*pinfo, sync, retcode);
			//通知客户端赠送失败
			SendErr( *pinfo , retcode);
			return;
		}

		DBPlayerGivePresent * rpc=(DBPlayerGivePresent *)Rpc::Call( RPC_DBPLAYERGIVEPRESENT, DBPlayerGivePresentArg(roleid, pinfo->userid, target_roleid, mail_id, goods, cash_cost, has_gift, log_price1, log_price2, pinfo->name, sync));
		rpc->save_linksid = pinfo->linksid;
		rpc->save_localsid = pinfo->localsid;
		rpc->save_gsid = pinfo->gameid;
		rpc->save_level = pinfo->level;
		GameDBClient::GetInstance()->SendProtocol(rpc);
		LOG_TRACE("PlayerGivePresent roleid %d send to gamedbd.", roleid);
	}
};

};

#endif
