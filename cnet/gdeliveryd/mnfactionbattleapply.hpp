
#ifndef __GNET_MNFACTIONBATTLEAPPLY_HPP
#define __GNET_MNFACTIONBATTLEAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"

#include "gproviderserver.hpp"
#include "mnfactionbattleapply_re.hpp"
namespace GNET
{

class MNFactionBattleApply : public GNET::Protocol
{
	#include "mnfactionbattleapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		int retcode = CDC_MNFactionBattleMan::GetInstance()->CheckMNFApply(fid, unifid, target);
		DEBUG_PRINT("CDC_MNFactionBattleMan[%s]fid=%d,unifid=%lld,roleid=%d,target=%d,cost=%d,retcode=%d:\n", __FILE__,fid, unifid, roleid, target, cost, retcode);
		if(retcode != ERR_SUCCESS)
		{
			//检查失败
			syncdata.inventory.items.clear();
			manager->Send(sid, GMailEndSync(0, retcode, roleid, syncdata));

			//通知客户端
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
			if(pinfo == NULL) return; 

			unsigned int linksid = pinfo->linksid;
			unsigned int localsid = pinfo->localsid;
			MNFactionBattleApply_Re re;
			re.retcode		= retcode;
			re.target		= target;
			re.localsid     = localsid;
			GDeliveryServer::GetInstance()->Send(linksid, re); 
			return;
		}
		else
		{
			DBMNFactionBattleApplyArg arg;
			arg.fid 		= fid;
			arg.unifid		= unifid;
			arg.zoneid		= GDeliveryServer::GetInstance()->GetZoneid();
			arg.roleid 		= roleid;
			arg.target 		= target;
			arg.cost 		= cost;
			arg.syncdata	= syncdata;
			GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBMNFACTIONBATTLEAPPLY, arg));
		}	
	}
};

};

#endif
