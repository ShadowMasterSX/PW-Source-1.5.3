
#ifndef __GNET_SYSRECOVEREDOBJMAIL_RE_HPP
#define __GNET_SYSRECOVEREDOBJMAIL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SysRecoveredObjMail_Re : public GNET::Protocol
{
	#include "sysrecoveredobjmail_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
