
#ifndef __GNET_MNGETPLAYERLASTENTERINFO_RE_HPP
#define __GNET_MNGETPLAYERLASTENTERINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetPlayerLastEnterInfo_Re : public GNET::Protocol
{
	#include "mngetplayerlastenterinfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG, "file=%s\tfunc=%s\tline=%d\tlocalsid=%d\n", __FILE__, __FUNCTION__, __LINE__, localsid);
		unsigned int tmp = localsid;
		this->localsid = 0;
		GLinkServer::GetInstance()->Send(tmp, this);
	}
};

};

#endif
