
#ifndef __GNET_CHATMULTICAST_HPP
#define __GNET_CHATMULTICAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "player"
#include "chatmessage.hpp"
#include <algorithm>
namespace GNET
{

class ChatMultiCast : public GNET::Protocol
{
	#include "chatmulticast"
	class DispatchData
	{
		const ChatMessage* chatmsg;
	public:
		DispatchData(const ChatMessage* msg):chatmsg(msg) {}
		~DispatchData() {}
		void operator() (Player& player)
		{
			GLinkServer::GetInstance()->Send(player.localsid,chatmsg);
		}	
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		ChatMessage chatmsg(channel,emotion,srcroleid,msg,data,srclevel);
		if  (playerlist.GetVector().size())
			std::for_each(playerlist.GetVector().begin(),playerlist.GetVector().end(),DispatchData(&chatmsg));	
	}
};

};

#endif
