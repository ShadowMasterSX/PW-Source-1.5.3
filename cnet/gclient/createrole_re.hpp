
#ifndef __GNET_CREATEROLE_RE_HPP
#define __GNET_CREATEROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#ifdef _TESTCODE //liping
	#include "makechoice.h"
#endif
namespace GNET
{

class CreateRole_Re : public GNET::Protocol
{
	#include "createrole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		//DEBUG_PRINT("Recv Protocol CreateRole_Re result=%d\n",result);
		if(result == ERR_SUCCESS)
		{
			GLinkClient* cm= (GLinkClient*)manager;
			cm->SetRole(roleid);
			if(cm->roleid)	cm->Send(sid,SelectRole( cm->roleid ));
		}
	}
};

};

#endif

