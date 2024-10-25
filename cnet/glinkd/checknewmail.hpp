
#ifndef __GNET_CHECKNEWMAIL_HPP
#define __GNET_CHECKNEWMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class CheckNewMail : public GNET::Protocol
{
	#include "checknewmail"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if ( !GLinkServer::ValidRole( sid,roleid ) )
			return;
		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol( this );
	}
};

};

#endif
