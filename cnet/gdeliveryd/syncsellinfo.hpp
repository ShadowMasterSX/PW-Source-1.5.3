
#ifndef __GNET_SYNCSELLINFO_HPP
#define __GNET_SYNCSELLINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
namespace GNET
{
class SyncSellInfo : public GNET::Protocol
{
	#include "syncsellinfo"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
