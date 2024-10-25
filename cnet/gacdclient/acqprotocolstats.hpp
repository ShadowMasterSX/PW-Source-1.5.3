
#ifndef __GNET_ACQPROTOCOLSTATS_HPP
#define __GNET_ACQPROTOCOLSTATS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "acq"
#include "acprotocolstat"

namespace GNET
{

class ACQProtocolStats : public GNET::Protocol
{
	#include "acqprotocolstats"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
