
#ifndef __GNET_FACTIONCHAT_HPP
#define __GNET_FACTIONCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gfactionclient.hpp"
#include "chatmessage.hpp"
namespace GNET
{

class FactionChat : public GNET::Protocol
{
	#include "factionchat"
	void SendForbidInfo(GLinkServer* lsm,Manager::Session::ID sid,const GRoleForbid& forbid)
	{
		lsm->Send(sid,AnnounceForbidInfo(src_roleid,_SID_INVALID,forbid));
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//转发帮派服务发来的聊天信息
		if (manager==GFactionClient::GetInstance())
		{
			if ( GLinkServer::IsRoleOnGame( dst_localsid ) )
			{
				if(channel!=GN_CHAT_CHANNEL_SYSTEM)
					GLinkServer::GetInstance()->Send(this->dst_localsid,this);
				else
				{
					ChatMessage chatmsg(GN_CHAT_CHANNEL_SYSTEM,0,src_roleid,msg,data);
					GLinkServer::GetInstance()->Send(dst_localsid,chatmsg);
				}
			}
		}
		//转发客户端发来的聊天信息
		else if(manager==GLinkServer::GetInstance())
		{
			if(msg.size()>256 || data.size()>8)
				return;
			GLinkServer* lsm=GLinkServer::GetInstance();
			SessionInfo * sinfo = lsm->GetSessionInfo(sid);
			if (!sinfo || sinfo->roleid!=src_roleid || src_roleid<=0 || !sinfo->policy.Update(CHAT_POLICY))
				return;
			GRoleForbid forbid;
			if (lsm->IsForbidChat(src_roleid, sinfo->userid, forbid))
			{
				SendForbidInfo(lsm, sid, forbid);
				return;
			}
			//GFactionClient::GetInstance()->SendProtocol(this);
			GProviderServer::GetInstance()->DispatchProtocol(sinfo->gsid,this);
		}
	}
};

};

#endif
