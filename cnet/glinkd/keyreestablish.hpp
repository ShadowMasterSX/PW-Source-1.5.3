
#ifndef __GNET_KEYREESTABLISH_HPP
#define __GNET_KEYREESTABLISH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class KeyReestablish : public GNET::Protocol
{
	#include "keyreestablish"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG, "CrossRelated Recv KeyReestablish roleid %d userid %d flag %d src_zoneid %d random.size %d sid %d", 
			roleid, userid, flag, src_zoneid, random.size(), sid);
		
		GLinkServer* lsm = (GLinkServer*)manager;
		
		lsm->halfloginset.insert(sid);
		if ( lsm->IsListening() && lsm->ExceedHalfloginLimit(lsm->halfloginset.size()) )
		{
			DEBUG_PRINT("glinkd::response:: halfloginuser exceed max number. PassiveIO closed. user size=%d\n",
				lsm->halfloginset.size());
		   	lsm->StopListen();
		}		

		if(!lsm->ValidSid(sid) || !random.size()) return;
		
		SessionInfo* sinfo = lsm->GetSessionInfo(sid);
		if(!sinfo) return;
		
		int client_ip = ((struct sockaddr_in*)(sinfo->GetPeer()))->sin_addr.s_addr;
		PlayerIdentityMatch* rpc = (PlayerIdentityMatch*)Rpc::Call(RPC_PLAYERIDENTITYMATCH, PlayerIdentityMatchArg(roleid, userid, client_ip, src_zoneid, random, flag, sid));
		
		if (!GDeliveryClient::GetInstance()->SendProtocol(rpc)) {
			//客户端已经将ISecKey设置 所以此处无法给客户端成功发送协议
			//lsm->SessionError(sid, ERR_COMMUNICATION, "Server Network Error.");
			lsm->Close(sid);
		} else {
			//LOG_TRACE("roleid %d change to state_GResponseReceive", roleid);
			lsm->ChangeState(sid, &state_GResponseReceive);
		}
	}
};

};

#endif
