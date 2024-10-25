
#ifndef __GNET_GETTASKDATA_RE_HPP
#define __GNET_GETTASKDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void get_task_data_reply(int taskid, int uid, const void * env_data, size_t env_size, const void * task_data, size_t task_size);
namespace GNET
{

class GetTaskData_Re : public GNET::Protocol
{
	#include "gettaskdata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if( retcode == 0 ) 
		{
			get_task_data_reply(taskid, playerid, env.begin(),env.size(),taskdata.begin(),taskdata.size());
		}
		else
			; // data not found
	}
};

};

#endif
