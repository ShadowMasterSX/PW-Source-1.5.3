
#ifndef __GNET_CHATMESSAGE_HPP
#define __GNET_CHATMESSAGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "conv_charset.h"
#include "vclient_if.h"
#include "glinkclient.hpp"

namespace GNET
{

class ChatMessage : public GNET::Protocol
{
	#include "chatmessage"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Octets msg_gbk;
		CharsetConverter::conv_charset_u2g(msg,msg_gbk);
		if(msg_gbk.size() > 1 && (*(char *)msg_gbk.begin()) == ':')
		{
			GLinkClient* cm = (GLinkClient*)manager;
			VCLIENT::RecvConsoleCmd(cm->roleid, srcroleid, (char *)msg_gbk.begin()+1, msg_gbk.size()-1);
		}
	}
};

};

#endif
