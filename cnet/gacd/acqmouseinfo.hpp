
#ifndef __GNET_ACQMOUSEINFO_HPP
#define __GNET_ACQMOUSEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acmouseinfo"
#include "userdata.hpp"

namespace GNET
{

class ACQMouseInfo : public GNET::Protocol
{
	#include "acqmouseinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}

	void operator() (const UserSessionData *data)
	{
		const std::deque<Mouse> &mouse = data->GetMouseInfo();
		for(std::deque<Mouse>::const_iterator it = mouse.begin(), ie = mouse.end(); it != ie; ++it)
		{
			const Mouse &m = (*it);
			ACMouseInfo info;
			info.lbuttonfreq = m.lbuttondownfreq;
			info.rbuttonfreq = m.rbuttondownfreq;
			info.mousemovefreq = m.mousemovefreq;
			info.mousemovevelo = m.mousemovevelo;
			for(int i=0; i< Mouse::RegionCount; ++i)
			{
				info.g_counts.push_back(IntData(m.g_counts[i]));
				info.g_times.push_back(IntData(m.g_times[i]));
			}
			mouse_info.push_back(info);
		}
	}
};

};

#endif
