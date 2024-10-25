
#ifndef __GNET_FACTIONCHAT_HPP
#define __GNET_FACTIONCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gfactionserver.hpp"
#include "base64.h"
#include "localmacro.h"
#include "factionmsg.hpp"

namespace GNET
{

class FactionChat : public GNET::Protocol
{
	#include "factionchat"
	struct Dispatch
	{
		FactionChat* p;
		Dispatch(FactionChat* _p) : p(_p) { }
		void operator() (const std::pair<unsigned int,HardReference<GFactionServer::Player> >& pair) {
			p->dst_localsid=pair.second->localsid;
			if(!Factiondb::GetInstance()->AlreadyDelayExpel(pair.second->roleid)) // ·ÇÑÓ³ÙÉ¾³ý
				GFactionServer::GetInstance()->DispatchProtocol(pair.second->linkid,p);
		}
	};
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GFactionServer* gfs=GFactionServer::GetInstance();
		unsigned int src_factionid=_FACTION_ID_INVALID;
		if(channel==GN_CHAT_CHANNEL_SYSTEM)
		{
			// For type=GN_CHAT_CHANNEL_SYSTEM, dst_localsid stands for faction id 
			DEBUG_PRINT("FactionChat: GN_CHAT_CHANNEL_SYSTEM, faction=%d type=%d\n", dst_localsid, src_roleid);
			std::for_each(gfs->factionmembermap.lower_bound(dst_localsid),
					gfs->factionmembermap.upper_bound(dst_localsid),
					Dispatch(this));
			return;
		}
		{
			Thread::Mutex::Scoped l(gfs->locker_player);
			if(Factiondb::GetInstance()->AlreadyDelayExpel(src_roleid)) // ·ÇÑÓ³ÙÉ¾³ý
				return;
			// For type=CHANNEL_GAMETALK, dst_localsid stands for faction id 
			GFactionServer::PlayerMap::const_iterator it=gfs->playermap.find(src_roleid);
			if (it!=gfs->playermap.end()) {
				src_factionid=(*it).second->faction_id;
				emotion = (*it).second->emotion;
			}

			if(channel == CHANNEL_GAMETALK) {
				src_factionid = dst_localsid;
			}

			if (src_factionid!=_FACTION_ID_INVALID && gfs->factionmembermap.count(src_factionid)!=0)
			{
				Octets out;
				Base64Encoder::Convert(out, msg);
				Log::log(LOG_CHAT,"Guild: src=%d fid=%d msg=%.*s",src_roleid,src_factionid,out.size(), 
						(char*)out.begin()); 
				std::for_each(gfs->factionmembermap.lower_bound(src_factionid),
						gfs->factionmembermap.upper_bound(src_factionid),
						Dispatch(this));

				if(channel != CHANNEL_GAMETALK) {
					GFactionServer *server = GFactionServer::GetInstance();
					FactionMsg fm;
					fm.factionid.factionid = src_factionid;
					fm.factionid.ftype = 0;
					fm.message.sender = (int64_t)src_roleid;
					fm.message.sendername =  it->second->name;
					fm.message.emotiongroup = emotion;
					fm.message.content.swap(msg);
					server->DispatchProtocol(0, fm);
				}
			}
		}
	}
};

};

#endif
