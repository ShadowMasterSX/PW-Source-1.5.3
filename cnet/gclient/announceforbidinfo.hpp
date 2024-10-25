
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
				dspType.replace("��½",4);
				break;
			case 101: 	//PRV_FORBID_TALK
				dspType.replace("����",4);
				break;
			case 102:
				dspType.replace("����",4);	
		}
		printf("����Ϊ%.*s�����%.*sȨ�ޣ�Ŀǰ���⻹��%d��\n",forbid.reason.size(),(char*) forbid.reason.begin(),
				dspType.size(),(char*) dspType.begin(),forbid.time);
		fflush(stdout);
	}
};

};

#endif
