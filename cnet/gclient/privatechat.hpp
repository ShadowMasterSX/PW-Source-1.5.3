
#ifndef __GNET_PRIVATECHAT_HPP
#define __GNET_PRIVATECHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "conv_charset.h"
namespace GNET
{

class PrivateChat : public GNET::Protocol
{
	#include "privatechat"

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
