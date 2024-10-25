
#ifndef __GNET_UNDODELETEROLE_RE_HPP
#define __GNET_UNDODELETEROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class UndoDeleteRole_Re : public GNET::Protocol
{
	#include "undodeleterole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (result!=ERR_SUCCESS)
		{
			Log::log(LOG_ERR,"glinkd:: undodeleterole_re: failed. result=%d\n",result);
		}
		GLinkServer* lsm=GLinkServer::GetInstance();
		lsm->ChangeState(localsid,&state_GSelectRoleServer);
		//send to client
		unsigned int tmpsid=localsid;
		localsid=_SID_INVALID;
		lsm->Send(tmpsid,this);
	}
};

};

#endif
