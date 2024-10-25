
#ifndef __GNET_PUBLICCHAT_HPP
#define __GNET_PUBLICCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gproviderclient.hpp"
#include "chatmulticast.hpp"

void handle_user_msg(int cs_index,int sid, int uid, const void * msg, size_t size, const void * aux_data, size_t size2, char channel);

namespace GNET
{

class PublicChat : public GNET::Protocol
{
	#include "publicchat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE
		DEBUG_PRINT("gamed::receive publicchat srcroleid=%d,localsid=%d,msg=%.*s\n",roleid,localsid,msg.size(),(char*)msg.begin());
		ChatMultiCast chatmulticast;
		chatmulticast.channel=channel;
		chatmulticast.srcroleid=roleid;
		chatmulticast.msg=msg;

		chatmulticast.playerlist.add(Player(roleid,localsid));
		chatmulticast.playerlist.add(Player(roleid,localsid));
		chatmulticast.playerlist.add(Player(roleid,localsid));
		chatmulticast.playerlist.add(Player(roleid,localsid));
		chatmulticast.playerlist.add(Player(roleid,localsid));
		manager->Send(sid,chatmulticast);
#endif
		handle_user_msg(((GProviderClient*)manager)->GetProviderServerID(),localsid,roleid,msg.begin(),msg.size(),data.begin(), data.size(),channel);
	}
};

};

#endif
