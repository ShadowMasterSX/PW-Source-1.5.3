
#ifndef __GNET_CHATBROADCAST_HPP
#define __GNET_CHATBROADCAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "chatmessage.hpp"
#include <algorithm>
namespace GNET
{

class ChatBroadCast : public GNET::Protocol
{
	#include "chatbroadcast"
	class DispatchData
	{
		const ChatMessage* chatmsg;
	public:
		DispatchData(const ChatMessage* msg):chatmsg(msg) {}
		~DispatchData() {}
		void operator() (std::pair<const int, GNET::RoleData> pair)
		{
			if (pair.second.status == _STATUS_ONGAME)
				GLinkServer::GetInstance()->Send(pair.second.sid,chatmsg);
		}	
	};
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		ChatMessage chatmsg(channel,emotion,srcroleid,msg,data);
		GLinkServer* lsm=GLinkServer::GetInstance();
		if ( lsm->roleinfomap.size() )
			std::for_each(lsm->roleinfomap.begin(),lsm->roleinfomap.end(),DispatchData(&chatmsg));
	}
};

};

#endif
