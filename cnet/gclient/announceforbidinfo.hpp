
#ifndef __GNET_ANNOUNCEFORBIDINFO_HPP
#define __GNET_ANNOUNCEFORBIDINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleforbid"
namespace GNET
{

class AnnounceForbidInfo : public GNET::Protocol
{
	#include "announceforbidinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("receive annouceforbidinfo: forbid.type=%d\n",forbid.type);
		Octets dspType;
		switch (forbid.type)
		{
			case 100:	//PRV_FORCE_OFFLINE
				dspType.replace("登陆",4);
				break;
			case 101: 	//PRV_FORBID_TALK
				dspType.replace("聊天",4);
				break;
			case 102:
				dspType.replace("交易",4);	
		}
		printf("你因为%.*s被封禁%.*s权限，目前离解封还有%d秒\n",forbid.reason.size(),(char*) forbid.reason.begin(),
				dspType.size(),(char*) dspType.begin(),forbid.time);
		fflush(stdout);
	}
};

};

#endif
