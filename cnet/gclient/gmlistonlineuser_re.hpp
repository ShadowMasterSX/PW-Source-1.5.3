
#ifndef __GNET_GMLISTONLINEUSER_RE_HPP
#define __GNET_GMLISTONLINEUSER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmplayerinfo"
#include "gmchoice.h"
#include "conv_charset.h"
#include "glinkclient.hpp"
namespace GNET
{

class GMListOnlineUser_Re : public GNET::Protocol
{
	#include "gmlistonlineuser_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("OnlineUsers (handler=%d):\n",handler);
		printf("%8s\t%8s\t%8s\t%8s\t%8s\t%8s\t%16s\n","userid","roleid","linkid","localsid","gsid","status","rolename");
		GLinkClient* lcm=GLinkClient::GetInstance();
		for (size_t i=0;i<userlist.size();i++)
		{
			Octets name_gbk;
			CharsetConverter::conv_charset_u2g(userlist[i].name,name_gbk);
			printf("%8d\t%8d\t%8d\t%8d\t%8d\t%8d\t%16.*s\n",
					userlist[i].userid,
					userlist[i].roleid,
					userlist[i].linkid,
					userlist[i].localsid,
					userlist[i].gsid,
					userlist[i].status,
					name_gbk.size(),
					(char*)name_gbk.begin());
			{
				Thread::Mutex::Scoped l(lcm->locker_rolenamemap);
				lcm->rolenamemap[userlist[i].roleid].swap(userlist[i].name);
			}
		}
		GMChoice(gmroleid,manager,sid);
	}
};

};

#endif
