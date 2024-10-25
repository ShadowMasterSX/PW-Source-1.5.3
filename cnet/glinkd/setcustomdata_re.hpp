
#ifndef __GNET_SETCUSTOMDATA_RE_HPP
#define __GNET_SETCUSTOMDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class SetCustomData_Re : public GNET::Protocol
{
	#include "setcustomdata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		unsigned int tmplocalsid=localsid;
		this->localsid=_SID_INVALID;
		GLinkServer::GetInstance()->Send(tmplocalsid,this);

	}
};

};

#endif
