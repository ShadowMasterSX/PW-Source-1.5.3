
#ifndef __GNET_ANNOUNCERESP_HPP
#define __GNET_ANNOUNCERESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gametalkmanager.h"
#include "gametalkclient.hpp"


namespace GNET
{

class AnnounceResp : public GNET::Protocol
{
	#include "announceresp"
	enum {
		RET_OK = 0,
		RET_INVALID_ZONE = 1,
		RET_DUPLICATE = 2,
		RET_MULTIZONE = 3
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		switch(code) {
		case RET_OK:
			GameTalkManager::GetInstance()->OnIdentification((time_t)boottime);
			break;
		case RET_INVALID_ZONE:
			Log::log(LOG_ERR, "GameTalk: zonedeliveryd doesn't serve this zone, check configuration, GameTalkClient closed");
			GameTalkClient::GetInstance()->ActiveClose();
			break;
		case RET_DUPLICATE:
			Log::log(LOG_ERR, "GameTalk: zonedeliveryd find duplicated GameTalkClient, check configuration, GameTalkClient closed");
			GameTalkClient::GetInstance()->ActiveClose();
			break;
		case RET_MULTIZONE:
			Log::log(LOG_ERR, "GameTalk: zonedeliveryd find multi AnnounceZoneidToIm with different zoneid, weired problem, GameTalkClient closed");
			GameTalkClient::GetInstance()->ActiveClose();
			break;
		default:
			Log::log(LOG_ERR, "GameTalk: zonedeliveryd response unknown code %d", code);
		}
	}
};

};

#endif
