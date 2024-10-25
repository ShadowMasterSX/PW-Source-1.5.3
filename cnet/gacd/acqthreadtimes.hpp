
#ifndef __GNET_ACQTHREADTIMES_HPP
#define __GNET_ACQTHREADTIMES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acthreadtime"
#include "userdata.hpp"

namespace GNET
{

class ACQThreadTimes : public GNET::Protocol
{
	#include "acqthreadtimes"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}

	bool operator() (const UserSessionData *data)
	{
		const ThreadTime& tt = data->GetProcessTime();
		process_time.thread_id = tt.tid;
		process_time.all_secs = tt.all_secs;
		process_time.kernel_secs = tt.kernel_secs;
		process_time.user_secs = tt.user_secs;
		const std::vector<ThreadTime> &tts = data->GetThreadTimes();
		thread_times.clear();
		for(std::vector<ThreadTime>::const_iterator it = tts.begin(), ie = tts.end(); it != ie; ++it)
		{
			const ThreadTime& ttt = (*it);
			thread_times.push_back(ACThreadTime(ttt.tid, ttt.all_secs, ttt.kernel_secs, ttt.user_secs));
		}
		return true;
	}
};

};

#endif
