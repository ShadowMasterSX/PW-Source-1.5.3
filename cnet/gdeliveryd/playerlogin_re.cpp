#include "playerlogin_re.hpp" 
#include "playerpositionresetrqst.hrp" 

namespace GNET
{
	void PlayerLogin_Re::ResetRequest(int userid, int linksid, int localsid)
	{
		PlayerPositionResetRqst* rpc = (PlayerPositionResetRqst*) Rpc::Call(RPC_PLAYERPOSITIONRESETRQST,PlayerPositionResetRqstArg(userid,roleid,localsid,0,PlayerPositionResetRqst::TransRecode(result)));
		rpc->CreateResetPos(); // create resetpos for player
		rpc->reason = result;
		rpc->src_provider_id = src_provider_id;
		rpc->sid = linksid;
		GDeliveryServer::GetInstance()->Send(linksid,rpc);
	}

}
