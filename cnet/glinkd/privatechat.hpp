
#ifndef __GNET_PRIVATECHAT_HPP
#define __GNET_PRIVATECHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "announceforbidinfo.hpp"
#include "base64.h"
namespace GNET
{

class PrivateChat : public GNET::Protocol
{
	#include "privatechat"
	void SendForbidInfo(GLinkServer* lsm,Manager::Session::ID sid,const GRoleForbid& forbid)
	{
		lsm->Send(sid,AnnounceForbidInfo(srcroleid,_SID_INVALID,forbid));
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm = GLinkServer::GetInstance();
		GDeliveryClient* dcm = GDeliveryClient::GetInstance();

		if ( srcroleid==dstroleid)
			return;
		
		if (manager == lsm) //msg come from client
		{
			if(msg.size()>256 || data.size()>8)
				return;
			SessionInfo * sinfo = lsm->GetSessionInfo(sid);
			if (!sinfo || sinfo->roleid!=srcroleid || srcroleid<=0 || !sinfo->policy.Update(WHISPER_POLICY))
				return;
			GRoleForbid forbid;
			if (lsm->IsForbidChat(srcroleid, sinfo->userid, forbid))
			{
				SendForbidInfo(lsm, sid, forbid);
				return;
			}
			//dcm->SendProtocol(this);  //send request to delivery
			//发送给发送者所在gs，获取其等级
			GProviderServer::GetInstance()->DispatchProtocol(sinfo->gsid,this);
		}
		else if (manager == dcm) //msg come from deliveryserver
		{
			GLinkServer::RoleMap::iterator it=lsm->roleinfomap.find(dstroleid);
			if (it!=lsm->roleinfomap.end()&&(*it).second.roleid==dstroleid&&(*it).second.status==_STATUS_ONGAME)
			{
				Octets out;
				Base64Encoder::Convert(out, msg);
				Log::log(LOG_CHAT, "Whisper: src=%d dst=%d msg=%.*s", srcroleid, dstroleid, out.size(), 
					(char*)out.begin()); 
				lsm->Send((*it).second.sid,this);
			}
		}
			
	}
};

};

#endif
