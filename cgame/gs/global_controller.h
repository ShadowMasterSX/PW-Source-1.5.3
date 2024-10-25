#ifndef __ONLINEGAME_GS_GLOBAL_CONTROLLER_H__
#define __ONLINEGAME_GS_GLOBAL_CONTROLLER_H__

#include <string>
#include <spinlock.h>

enum
{
	SERVER_FORBID_CTRL = 1,
	SERVER_FORBID_ITEM = 2,
	SERVER_FORBID_SERVICE = 3,
	SERVER_FORBID_TASK = 4,
	SERVER_FORBID_SKILL = 5,
    SERVER_FORBID_SHOPITEM = 7,
    SERVER_FORBID_RECIPE = 8,
};


class GlobalController
{
	int _lock;
	std::string _conf_filename;
	time_t _conf_filemtime;

	bool _cash_money_exchange_open;
	int _cash_money_exchange_rate;	//元宝金币兑换比率
	std::vector<int> _forbid_ctrl_list;
	std::vector<int> _forbid_item_list;
	std::vector<int> _forbid_service_list;
	std::vector<int> _forbid_task_list;
	std::vector<int> _forbid_skill_list;
    std::vector<int> _forbid_shopitem_list;
    std::vector<int> _forbid_recipe_list;

	void __Clear();
	void __Reload();
public:
	GlobalController():_lock(0),_conf_filemtime(0),_cash_money_exchange_open(false),_cash_money_exchange_rate(0)
	{
	}
	
	bool Init(const char * file);
	void CheckUpdate();

	bool GetCashMoneyExchangeOpen(){ spin_autolock keeper(_lock); return _cash_money_exchange_open; }
	int GetCashMoneyExchangeRate(){ spin_autolock keeper(_lock); return _cash_money_exchange_rate; }
	void SetCashMoneyExchangeRate(bool open, int rate);
	void SetServerForbid(std::vector<int> &ctrl_list,std::vector<int> &item_list,std::vector<int> &service_list,std::vector<int> &task_list,std::vector<int> &skill_list, std::vector<int> &shopitem_list, std::vector<int>& recipe_list);
	bool CheckServerForbid(int type,int id);
	void SetServerTrigger(std::vector<int> &trigger_list);
};

#endif
