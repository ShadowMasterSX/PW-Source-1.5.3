
#ifndef __GNET_GMCONTROLGAME_HPP
#define __GNET_GMCONTROLGAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gmcontrolgame_re.hpp"

bool gm_control_command(int tag, const char * buf);
namespace GNET
{

class GMControlGame : public GNET::Protocol
{
	#include "gmcontrolgame"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GMControlGame_Re re;
		re.xid = xid;
		char * buf = new char[command.size() + 1];
		memcpy(buf, command.begin(), command.size());
		buf[command.size()] = 0;
		re.retcode = gm_control_command(worldtag, buf) ? 0 : 1;
		delete [] buf;
		manager->Send(sid, re);
	}
};

};

#endif
