
#ifndef __GNET_ERRORINFO_HPP
#define __GNET_ERRORINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkclient.hpp"

namespace GNET
{

class ErrorInfo : public GNET::Protocol
{
	#include "errorinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkClient * cm = (GLinkClient *)manager;
		DEBUG_PRINT("Recv Protocol ErrorInfo. user %d role %d err %d(%.*s)\n",cm->userid,cm->roleid,errcode,info.size(),(char*)info.begin());
		fflush(stdout);
	}
};

};

#endif
