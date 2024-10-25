
#ifndef __GNET_SYNCFORCEGLOBALDATA_HPP
#define __GNET_SYNCFORCEGLOBALDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gforceglobaldatabrief"

void UpdateForceGlobalData(int force_id, int player_count, int development, int construction, int activity, int activity_level);

namespace GNET
{

class SyncForceGlobalData : public GNET::Protocol
{
	#include "syncforceglobaldata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if(!list.size())
		{
			UpdateForceGlobalData(0,0,0,0,0,0);
			return;
		}
		for(size_t i=0; i<list.size(); i++)
		{
			GForceGlobalDataBrief & data = list[i];
			UpdateForceGlobalData(data.force_id, data.player_count, data.development, data.construction, data.activity, data.activity_level);
		}
	}
};

};

#endif
