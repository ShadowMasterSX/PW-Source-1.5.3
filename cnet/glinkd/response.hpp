
#ifndef __GNET_RESPONSE_HPP
#define __GNET_RESPONSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "matrixpasswd.hrp"
#include "matrixpasswd2.hrp"
namespace GNET
{

class Response : public GNET::Protocol
{
	#include "response"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT ("glinkd::receive response from client. identity=%.*s(%d),sid=%d\n",
			identity.size(),(char*)identity.begin(),identity.size(),sid);
		GLinkServer *lsm = (GLinkServer *)manager;
		if (!lsm->ValidSid(sid) || !identity.size())
			return;
		SessionInfo * sinfo = lsm->GetSessionInfo(sid);
		if (!sinfo)
			return;
			
		Rpc * rpc = NULL;
		int client_ip = ((struct sockaddr_in*)(sinfo->GetPeer()))->sin_addr.s_addr;
		char algo = lsm->challenge_algo;
		if(use_token)
		{
			rpc = Rpc::Call(RPC_MATRIXTOKEN, MatrixTokenArg(identity,response,client_ip,sinfo->challenge));
			((MatrixToken*)rpc)->save_sid = sid;
		}
		else if(lsm->GetAUVersion() == 0)
		{
			if (algo == ALGO_SHA)
				rpc = Rpc::Call(RPC_MATRIXPASSWD, MatrixPasswdArg(identity,response,client_ip));
			else
				rpc = Rpc::Call(RPC_MATRIXPASSWD, MatrixPasswdArg(identity, sinfo->challenge, client_ip));
			((MatrixPasswd*)rpc)->save_sid = sid;
		}
		else //GetAUVersion() == 1
		{
			if (algo == ALGO_SHA)
				rpc = Rpc::Call(RPC_MATRIXPASSWD2, MatrixPasswdArg(identity,response,client_ip));
			else
				rpc = Rpc::Call(RPC_MATRIXPASSWD2, MatrixPasswdArg(identity, sinfo->challenge, client_ip));
			((MatrixPasswd2*)rpc)->save_sid = sid;
		}

		sinfo->identity.swap(identity);
		sinfo->response.swap(response);
		sinfo->cli_fingerprint.swap(cli_fingerprint);
		
		lsm->halfloginset.insert(sid);
		if ( lsm->IsListening() && lsm->ExceedHalfloginLimit(lsm->halfloginset.size()) )
		{
			DEBUG_PRINT("glinkd::response:: halfloginuser exceed max number. PassiveIO closed. user size=%d\n",
				lsm->halfloginset.size());
		   	lsm->StopListen();
		}
		if (!GDeliveryClient::GetInstance()->SendProtocol(rpc))
		{
			lsm->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
		}
		else
		{
			lsm->ChangeState(sid,&state_GResponseReceive);
		}
	}
};

};

#endif
