
#ifndef __GNET_IWEBAUTOLOCKGET_RE_HPP
#define __GNET_IWEBAUTOLOCKGET_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class IWebAutolockGet_Re : public GNET::Protocol
{
	#include "iwebautolockget_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
