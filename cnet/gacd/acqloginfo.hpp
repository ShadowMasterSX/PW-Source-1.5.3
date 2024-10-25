
#ifndef __GNET_ACQLOGINFO_HPP
#define __GNET_ACQLOGINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acq"
#include "acloginfo"
#include "userdata.hpp"

namespace GNET
{

class ACQLogInfo : public GNET::Protocol
{
	#include "acqloginfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}

	bool operator() (const UserData *data)
	{
		logs = data->GetLogs();
		return true;
	}
};

};

#endif
