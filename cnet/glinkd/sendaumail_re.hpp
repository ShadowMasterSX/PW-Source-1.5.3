
#ifndef __GNET_SENDAUMAIL_RE_HPP
#define __GNET_SENDAUMAIL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SendAUMail_Re : public GNET::Protocol
{
	#include "sendaumail_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//if (!GLinkServer::ValidRole(localsid,roleid)) 
		//	return;
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp,this);	
	}
};

};

#endif
