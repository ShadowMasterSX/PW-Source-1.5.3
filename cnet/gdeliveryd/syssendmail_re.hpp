
#ifndef __GNET_SYSSENDMAIL_RE_HPP
#define __GNET_SYSSENDMAIL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SysSendMail_Re : public GNET::Protocol
{
	#include "syssendmail_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
