
#ifndef __GNET_SERVERFORBIDNOTIFY_HPP
#define __GNET_SERVERFORBIDNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void notify_serverforbid(std::vector<int> &ctrl_list,std::vector<int> &item_list,std::vector<int> &service_list,std::vector<int> &task_list,std::vector<int> &skill_list, std::vector<int> &shopitem_list, std::vector<int>& recipe_list);

namespace GNET
{

class ServerForbidNotify : public GNET::Protocol
{
	#include "serverforbidnotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		notify_serverforbid(forbid_ctrl_list,forbid_item_list,forbid_service_list,forbid_task_list,forbid_skill_list, forbid_shopitem_list, forbid_recipe_list);
	}
};

};

#endif
