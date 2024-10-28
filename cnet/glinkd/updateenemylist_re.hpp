
#ifndef __GNET_UPDATEENEMYLIST_RE_HPP
#define __GNET_UPDATEENEMYLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "genemylist"

namespace GNET
{

class UpdateEnemyList_Re : public GNET::Protocol
{
	#include "updateenemylist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        unsigned int tmp = localsid;
        this->localsid = _SID_INVALID;
        GLinkServer::GetInstance()->Send(tmp, this);
	}
};

};

#endif
