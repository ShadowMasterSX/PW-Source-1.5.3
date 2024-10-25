
#ifndef __GNET_UNDODELETEROLE_RE_HPP
#define __GNET_UNDODELETEROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "makechoice.h"
namespace GNET
{

class UndoDeleteRole_Re : public GNET::Protocol
{
	#include "undodeleterole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (result == ERR_SUCCESS)
		{
			DEBUG_PRINT("Client::Undo delete role %d successfully.\n",roleid);
		}
		else
		{
			DEBUG_PRINT("Client::Undo delete role %d failed.\n",roleid);
		}
#ifdef _TESTCODE
		MakeChoice(((GLinkClient*)manager)->userid,manager,sid);
#endif
	}
};

};

#endif
