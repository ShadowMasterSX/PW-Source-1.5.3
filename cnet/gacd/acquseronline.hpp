
#ifndef __GNET_ACQUSERONLINE_HPP
#define __GNET_ACQUSERONLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acq"
#include "intdata"
#include "octetsdata"
#include "userdata.hpp"

namespace GNET
{

class ACQUserOnline : public GNET::Protocol
{
	#include "acquseronline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}

	void operator() (const UserSessionData *data)
	{
		users.push_back(IntData(data->GetUserID()));
	}
};

};

#endif
