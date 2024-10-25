
#ifndef __GNET_CHATROOMLIST_RE_HPP
#define __GNET_CHATROOMLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gchatroom"

namespace GNET
{

class ChatRoomList_Re : public GNET::Protocol
{
	#include "chatroomlist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		unsigned int id = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(id,this);	
	}
};

};

#endif
