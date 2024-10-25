
#ifndef __GNET_KEYEXCHANGE_HPP
#define __GNET_KEYEXCHANGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"
#include "security.h"
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
		void Process(Manager *manager, Manager::Session::ID sid)
		{
			// TODO
			GLinkClient *cm=(GLinkClient*)manager;
			//compute key OUT  //set output Security (client)
			Octets keyout;
			cm->SetOSecurity(sid,ARCFOURSECURITY,GenerateKey(cm->identity,cm->password,nonce,keyout));

			//computer Key IN
			Security *random = Security::Create(RANDOM);
			random->Update(nonce.resize(16));
			random->Destroy();
			blkickuser = true;
			cm->SendProtocol(this);
			//set Input Security (client)
			Octets keyin;
			cm->SetISecurity(sid,DECOMPRESSARCFOURSECURITY,GenerateKey(cm->identity,cm->password,nonce,keyin));

			//DEBUG_PRINT("Recv Protocol KeyExchange.\n");
		}
};

};

#endif
