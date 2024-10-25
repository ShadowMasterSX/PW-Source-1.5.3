
#ifndef __GNET_PUBLICCHAT_HPP
#define __GNET_PUBLICCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
#include "chatsinglecast.hpp"
#include "chatbroadcast.hpp"
#include "trade.h"
#include "maplinkserver.h"
#include "mapuser.h"
#include "countrybattleman.h"
namespace GNET
{

class PublicChat : public GNET::Protocol
{
	#include "publicchat"
	typedef std::map<int/*linksid*/, PlayerVector> cs_player_map;

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("PublicChat: challel=%d roleid=%d emotion=%d", channel, roleid, emotion);
		switch (channel)
		{
			case GN_CHAT_CHANNEL_BROADCAST:	
				{
				//privilege check
				ChatBroadCast cbc(channel,0,roleid,msg,Octets());
				LinkServer::GetInstance().BroadcastProtocol( cbc );
				}
				break;
			case GN_CHAT_CHANNEL_WHISPER:
				break;
			case GN_CHAT_CHANNEL_COUNTRY:
				{
					const cs_player_map* pList = CountryBattleMan::GetInstance()->GetCountryOnlinePlayers(roleid);
					if(!pList) break;
					ChatMultiCast chat;
					chat.channel = channel;
					chat.emotion = emotion;
					chat.srcroleid = roleid;
					chat.msg = msg;
					chat.data = data;
					chat.srclevel = 0;
					for(cs_player_map::const_iterator it=pList->begin(); it!=pList->end(); ++it)
					{
						chat.playerlist = it->second;		
						GDeliveryServer::GetInstance()->Send(it->first, chat);
					}
				}
				break;
		}

	}
};

};

#endif
