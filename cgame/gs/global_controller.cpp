#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dbgprt.h"
#include "string.h"
#include "world.h"
#include "common/message.h"

#include <conf.h>
#include <glog.h>
#include "worldmanager.h"
#include "global_controller.h"


void GlobalController::__Clear()
{
}

void GlobalController::__Reload()
{
	__Clear();
	//ONET::Conf conf(_conf_filename.c_str());
}

bool GlobalController::Init(const char * filename)
{
	if(access(filename, R_OK) == 0)
	{
		_conf_filename = filename;
		struct stat st;
		stat(_conf_filename.c_str(), &st);
		_conf_filemtime = st.st_mtime;
		__Reload();
		return true;
	}
	return false;
}

void GlobalController::CheckUpdate()
{
	if(access(_conf_filename.c_str(), R_OK) == 0)
	{
		struct stat st;
		stat(_conf_filename.c_str(), &st);
		spin_autolock keeper(_lock);
		while(st.st_mtime != _conf_filemtime)
		{
			__Reload();
			_conf_filemtime = st.st_mtime;
			stat(_conf_filename.c_str(), &st);
		}	
	}
}

void GlobalController::SetCashMoneyExchangeRate(bool open, int rate)
{
	if(open && (rate < 500000 || rate > 3000000)) return;

	GLog::log(GLOG_INFO, "设置元宝金币兑换比率: worldtag=%d open=%d rate=%d", world_manager::GetWorldTag(), open?1:0, rate);
	spin_autolock keeper(_lock);
	_cash_money_exchange_open = open;
	_cash_money_exchange_rate = rate;
}

void GlobalController::SetServerForbid(std::vector<int> &ctrl_list,std::vector<int> &item_list,std::vector<int> &service_list,std::vector<int> &task_list,std::vector<int> &skill_list, std::vector<int> &shopitem_list, std::vector<int>& recipe_list)
{
	spin_autolock keeper(_lock);
	_forbid_ctrl_list.swap(ctrl_list);
	for(size_t i=0; i<_forbid_ctrl_list.size(); i++)
	{
		world_manager::GetInstance()->ClearSpawn(_forbid_ctrl_list[i]);
	}

	_forbid_item_list.swap(item_list);
	_forbid_service_list.swap(service_list);
	_forbid_task_list.swap(task_list);
	_forbid_skill_list.swap(skill_list);
    _forbid_shopitem_list.swap(shopitem_list);
    _forbid_recipe_list.swap(recipe_list);
}

bool GlobalController::CheckServerForbid(int type,int id)
{
	spin_autolock keeper(_lock);
	switch(type)
	{
	case SERVER_FORBID_CTRL:
		for(size_t i=0; i<_forbid_ctrl_list.size(); i++)
		{
			if(id == _forbid_ctrl_list[i])
				return true;
		}
		break;
	case SERVER_FORBID_ITEM:
		for(size_t i=0; i<_forbid_item_list.size(); i++)
		{
			if(id == _forbid_item_list[i])
				return true;
		}
		break;
	case SERVER_FORBID_SERVICE:
		for(size_t i=0; i<_forbid_service_list.size(); i++)
		{
			if(id == _forbid_service_list[i])
				return true;
		}
		break;
	case SERVER_FORBID_TASK:
		for(size_t i=0; i<_forbid_task_list.size(); i++)
		{
			if(id == _forbid_task_list[i])
				return true;
		}
		break;
	case SERVER_FORBID_SKILL:
		for(size_t i=0; i<_forbid_skill_list.size(); i++)
		{
			if(id == _forbid_skill_list[i])
				return true;
		}
		break;
    case SERVER_FORBID_SHOPITEM:
		for(size_t i=0; i<_forbid_shopitem_list.size(); i++)
		{
			if(id == _forbid_shopitem_list[i])
				return true;
		}
		break;
    case SERVER_FORBID_RECIPE:
        for (size_t i = 0; i < _forbid_recipe_list.size(); ++i)
        {
            if (id == _forbid_recipe_list[i])
                return true;
        }
        break;
	default:
		break;
	}
	return false;
}

void GlobalController::SetServerTrigger(std::vector<int> &trigger_list)
{
	for(size_t i=0;i<trigger_list.size(); i++)
	{
		world_manager::GetInstance()->TriggerSpawn(trigger_list[i]);
	}
}


