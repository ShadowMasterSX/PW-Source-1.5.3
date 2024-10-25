
#ifndef __GNET_FACTIONRENAMEGSVERIFY_RE_HPP
#define __GNET_FACTIONRENAMEGSVERIFY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"

namespace GNET
{

class FactionRenameGsVerify_Re : public GNET::Protocol
{
	#include "factionrenamegsverify_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
