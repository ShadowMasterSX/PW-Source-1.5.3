
#ifndef __GNET_RESPONSE_HPP
#define __GNET_RESPONSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "security.h"
namespace GNET
{

class Response : public GNET::Protocol
{
	#include "response"

	void Setup(Octets& name,Octets& passwd,Octets challenge)
	{
		HMAC_MD5Hash hash;
		MD5Hash md5;
		Octets digest;
		
		md5.Update(name);
		md5.Update(passwd);
		md5.Final(digest);
		
		hash.SetParameter(digest);
		hash.Update(challenge);
		hash.Final(response);

		identity.replace(name.begin(),name.size());
		use_token = 0;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
