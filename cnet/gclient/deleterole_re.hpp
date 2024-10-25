
#ifndef __GNET_DELETEROLE_RE_HPP
#define __GNET_DELETEROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE  //liping
	#include "makechoice.h"
#endif
namespace GNET
{

class DeleteRole_Re : public GNET::Protocol
{
	#include "deleterole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (result == ERR_SUCCESS)
		{
			DEBUG_PRINT("client::delete role %d successfully.\n",roleid);
		}
		else
		{
			DEBUG_PRINT("client::delete role %d failed.\n",roleid);
		}
#ifdef _TESTCODE
		MakeChoice(((GLinkClient*)manager)->userid,manager,sid);
#endif
	}
};

};

#endif
