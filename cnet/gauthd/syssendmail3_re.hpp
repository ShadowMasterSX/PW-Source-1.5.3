
#ifndef __GNET_SYSSENDMAIL3_RE_HPP
#define __GNET_SYSSENDMAIL3_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SysSendMail3_Re : public GNET::Protocol
{
	#include "syssendmail3_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
