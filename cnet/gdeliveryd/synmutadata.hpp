
#ifndef __GNET_SYNMUTADATA_HPP
#define __GNET_SYNMUTADATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
#include "localmacro.h"

namespace GNET
{

class SynMutaData : public GNET::Protocol
{
	#include "synmutadata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo *pinfo = UserContainer::GetInstance().FindRole(roleid);
		if(pinfo == NULL || (unsigned int )pinfo->roleid != roleid || synmask == SYNMUTADATAMASK_NONE)
		{
			return;
		}
		if(synmask & SYNMUTADATAMASK_LEVEL)
		{
			pinfo->level = level;
			pinfo->reincarnation_times = reincarnation_times;
		}
		return;
	}
};

};

#endif
