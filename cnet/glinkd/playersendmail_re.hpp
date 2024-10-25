
#ifndef __GNET_PLAYERSENDMAIL_RE_HPP
#define __GNET_PLAYERSENDMAIL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class PlayerSendMail_Re : public GNET::Protocol
{
	#include "playersendmail_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
