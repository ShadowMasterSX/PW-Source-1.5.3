
#ifndef __GNET_FACTIONRESOURCEBATTLEGETRECORD_RE_HPP
#define __GNET_FACTIONRESOURCEBATTLEGETRECORD_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionresourcebattlerecord"

namespace GNET
{

class FactionResourceBattleGetRecord_Re : public GNET::Protocol
{
	#include "factionresourcebattlegetrecord_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid, this);
	}
};

};

#endif
