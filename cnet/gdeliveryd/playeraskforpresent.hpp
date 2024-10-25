
#ifndef __GNET_PLAYERASKFORPRESENT_HPP
#define __GNET_PLAYERASKFORPRESENT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbplayeraskforpresent.hrp"


namespace GNET
{

class PlayerAskForPresent : public GNET::Protocol
{
	#include "playeraskforpresent"
	
	void SendErr(const PlayerInfo & pinfo, int retcode, int roleid)
	{
		GDeliveryServer::GetInstance()->Send(
				pinfo.linksid, 
				PlayerAskForPresent_Re(pinfo.localsid, roleid, retcode)
				);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("PlayerAskForPresent roleid=%d targetroleid=%d goods_id=%d goods_index=%d goods_slot=%d",
				roleid,target_roleid,goods_id,goods_index,goods_slot);

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo ) return;
		
		GDeliveryServer * dsm=GDeliveryServer::GetInstance(); 
		if (dsm->IsCentralDS())
		{
			SendErr( *pinfo, ERR_GP_OUTOFSERVICE, roleid);
			return;
		}
		
		if(PostOffice::GetInstance().GetMailBoxSize(target_roleid) >= SYS_MAIL_LIMIT)
		{
			SendErr( *pinfo, ERR_GP_ASK_TARGET_MAILBOX_FULL, roleid);
			return;
		}

		DBPlayerAskForPresent * rpc = (DBPlayerAskForPresent *)Rpc::Call( RPC_DBPLAYERASKFORPRESENT, 
				DBPlayerAskForPresentArg(roleid, target_roleid, goods_id, goods_index, goods_slot, pinfo->name));
		rpc->save_linksid = pinfo->linksid;
		rpc->save_localsid = pinfo->localsid;
		rpc->save_gsid = pinfo->gameid;
		GameDBClient::GetInstance()->SendProtocol(rpc);
		LOG_TRACE("PlayerAskForPresent roleid %d send to gamedbd.", roleid);
	}
};

};

#endif
