
#ifndef __GNET_ADDFRIENDREMARKS_RE_HPP
#define __GNET_ADDFRIENDREMARKS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AddFriendRemarks_Re : public GNET::Protocol
{
	#include "addfriendremarks_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        unsigned int tmp_sid = srclsid;
        this->srclsid = _SID_INVALID;

        Log::log(LOG_INFO, "addfriendremarks: glinkd. tmp_sid = %d, sid = %d.\n", tmp_sid, sid);

        GLinkServer::GetInstance()->Send(tmp_sid, this);
	}
};

};

#endif
