
#ifndef __GNET_PUBLICCHAT_HPP
#define __GNET_PUBLICCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryclient.hpp"
#include "announceforbidinfo.hpp"
#include "privilege.hxx"
#include "base64.h"
namespace GNET
{

class PublicChat : public GNET::Protocol
{
	#include "publicchat"
	void SendForbidInfo(GLinkServer* lsm,Manager::Session::ID sid,const GRoleForbid& forbid)
	{
		lsm->Send(sid,AnnounceForbidInfo(roleid,_SID_INVALID,forbid));
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(msg.size()>256 || data.size()>8)
			return;
		GLinkServer* lsm=GLinkServer::GetInstance();
		SessionInfo * sinfo = lsm->GetSessionInfo(sid);
		if (!sinfo) 
			return;
		if (sinfo->roleid != roleid || roleid<=0 || !sinfo->policy.Update(CHAT_POLICY))
			return;

		GRoleForbid forbid;
		if (lsm->IsForbidChat(roleid, sinfo->userid, forbid))
		{
			SendForbidInfo(lsm, sid, forbid);
			return;
		}
		Octets out;
		Base64Encoder::Convert(out, msg);
		Log::log(LOG_CHAT, "Chat: src=%d chl=%d msg=%.*s", roleid, channel, out.size(), (char*)out.begin()); 
		
		localsid = sid;

		switch (channel)
		{
			case GN_CHAT_CHANNEL_LOCAL:
			case GN_CHAT_CHANNEL_TRADE:
				sinfo->protostat.publicchat++;
			case GN_CHAT_CHANNEL_TEAM:
			case GN_CHAT_CHANNEL_FARCRY:
			case GN_CHAT_CHANNEL_SUPERFARCRY:
			case GN_CHAT_CHANNEL_BATTLE:
			case GN_CHAT_CHANNEL_COUNTRY:
				GProviderServer::GetInstance()->DispatchProtocol(sinfo->gsid,this);
				break;
			case GN_CHAT_CHANNEL_BROADCAST:	
				//privilege check
				if (! lsm->PrivilegeCheck(sid, roleid,Privilege::PRV_BROADCAST) )
				{
					Log::log(LOG_ERR,"WARNING: user %d is not allowed to use GM_OP_BROADCAST.\n", roleid);
					return;
				}
				else
				{
					Log::gmoperate(roleid,Privilege::PRV_BROADCAST,"Broadcast");
				}
				GDeliveryClient::GetInstance()->SendProtocol(this);
				break;	
		}
	}
};

};

#endif
