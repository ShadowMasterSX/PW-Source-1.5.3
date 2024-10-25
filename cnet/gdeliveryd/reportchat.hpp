
#ifndef __GNET_REPORTCHAT_HPP
#define __GNET_REPORTCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "base64.h"

namespace GNET
{

class ReportChat : public GNET::Protocol
{
	#include "reportchat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if ( NULL!=pinfo )
		{
			Octets name_base64, dst_name_base64, content_base64;
			if(pinfo->name.size()) Base64Encoder::Convert(name_base64, pinfo->name);
			if(dst_rolename.size()) Base64Encoder::Convert(dst_name_base64, dst_rolename);
			if(content.size()) Base64Encoder::Convert(content_base64, content);
			Log::formatlog("reportchat","zoneid=%d:roleid=%d:rolename=%.*s:dst_roleid=%d:dst_rolename=%.*s:content=%.*s",
					(unsigned char)(GDeliveryServer::GetInstance()->zoneid), roleid, name_base64.size(), name_base64.begin(), dst_roleid, dst_name_base64.size(), dst_name_base64.begin(), content_base64.size(), content_base64.begin());
		}
	}
};

};

#endif
