
#ifndef __GNET_CHATROOMJOIN_RE_HPP
#define __GNET_CHATROOMJOIN_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gchatmember"
#include "groomdetail"

namespace GNET
{

class ChatRoomJoin_Re : public GNET::Protocol
{
	#include "chatroomjoin_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int id = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(id,this);	
	}
};

};

#endif
