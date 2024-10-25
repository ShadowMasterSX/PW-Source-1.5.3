
#ifndef __GNET_KEYEXCHANGE_HPP
#define __GNET_KEYEXCHANGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "userlogin.hrp"
#include "userlogin2.hrp"

namespace GNET
{

class KeyExchange : public GNET::Protocol
{
	#include "keyexchange"

private:
	Octets& GenerateKey(Octets &identity, Octets &password, Octets &nonce, Octets &key)
	{
		HMAC_MD5Hash hash;
		hash.SetParameter(identity);
		hash.Update(password);
		hash.Update(nonce);
		return hash.Final(key);
	}

public:
	void Setup(Octets &identity, Octets &password, Manager *manager, Manager::Session::ID sid)
	{
		Security *random = Security::Create(RANDOM);
		random->Update(nonce.resize(16));
		random->Destroy();

		Octets key;
		GLinkServer::GetInstance()->SetSaveISecurity(sid, ARCFOURSECURITY, GenerateKey(identity, password, nonce, key));
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer *lsm = (GLinkServer *)manager;
		
		SessionInfo * sinfo = lsm->GetSessionInfo(sid);
		if (!sinfo) return;
		
		Octets key;
		lsm->SetSaveOSecurity(sid,COMPRESSARCFOURSECURITY,GenerateKey(sinfo->identity,sinfo->response,nonce,key));
		
		Rpc *rpc = NULL;
		int client_ip = ((struct sockaddr_in*)(sinfo->GetPeer()))->sin_addr.s_addr;
		if(lsm->GetAUVersion() == 0)
		{
			UserLoginArg arg(sinfo->userid, sid, blkickuser, 0, client_ip, sinfo->identity, Octets(), Octets());
			lsm->GetSecurityKeys(sid, arg.iseckey, arg.oseckey);
			
			rpc = Rpc::Call(RPC_USERLOGIN, arg);
			((UserLogin *)rpc)->localsid = sid;
		}
		else //GetAUVersion() == 1
		{
			UserLogin2Arg arg(sinfo->userid, sid, blkickuser, 0, sinfo->used_elec_number, 0, Octets(), client_ip, sinfo->identity, Octets(), Octets());
			lsm->GetSecurityKeys(sid, arg.iseckey, arg.oseckey);

			rpc = Rpc::Call(RPC_USERLOGIN2, arg);
			((UserLogin2 *)rpc)->localsid = sid;
		}
		
		if ( GDeliveryClient::GetInstance()->SendProtocol(rpc) )
		{
			lsm->ChangeState(sid,&state_GKeyExchgSucc);
		}
		else
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
