
#ifndef __GNET_WORLDCHAT_HPP
#define __GNET_WORLDCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class WorldChat : public GNET::Protocol
{
	#include "worldchat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Octets msg_gbk;
		CharsetConverter::conv_charset_u2g(msg,msg_gbk);
		if(msg_gbk.size() > 1 && (*(char *)msg_gbk.begin()) == ':')
		{
			GLinkClient* cm = (GLinkClient*)manager;
			VCLIENT::RecvConsoleCmd(cm->roleid, roleid, (char *)msg_gbk.begin()+1, msg_gbk.size()-1);
		}
	}
};

};

#endif
