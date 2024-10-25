
#ifndef __GNET_PLAYERRENAME_RE_HPP
#define __GNET_PLAYERRENAME_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gsp_if.h"


namespace GNET
{

class PlayerRename_Re : public GNET::Protocol
{
	#include "playerrename_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (retcode == ERR_SUCCESS)
		{
			player_rename_ret(roleid, newname.begin(), newname.size(), retcode);
		}
	}
};

};

#endif
