
#ifndef __GNET_MNFACTIONBATTLEAPPLY_RE_HPP
#define __GNET_MNFACTIONBATTLEAPPLY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNFactionBattleApply_Re : public GNET::Protocol
{
	#include "mnfactionbattleapply_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG, "file=%s\tfunc=%s\tline=%d\tretcode=%dlocalsid=%d\n", __FILE__, __FUNCTION__, __LINE__, retcode, localsid);
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp, this);
	}
};

};

#endif
