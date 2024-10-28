
#ifndef __GNET_MNGETTOPLIST_HPP
#define __GNET_MNGETTOPLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "cdcmnfbattleman.h"
namespace GNET
{

class MNGetTopList : public GNET::Protocol
{
	#include "mngettoplist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		CDC_MNToplistMan::GetInstance()->SendClientToplist(roleid);
	}
};

};

#endif
