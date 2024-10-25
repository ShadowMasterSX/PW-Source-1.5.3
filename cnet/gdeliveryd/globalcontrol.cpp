#include "gproviderserver.hpp"
#include "dbloadglobalcontrol.hrp"
#include "dbputglobalcontrol.hrp"
#include "cashmoneyexchangenotify.hpp"
#include "globalcontrol.h"
#include "serverforbidnotify.hpp"
#include "servertriggernotify.hpp"
#include "base64.h"
#include <string>
#include <algorithm>
namespace GNET
{

void GlobalControl::OnDBConnect(Protocol::Manager * manager, int sid)
{
	if(!_initialized)
		manager->Send(sid, Rpc::Call(RPC_DBLOADGLOBALCONTROL, DBLoadGlobalControlArg()));	
}

void GlobalControl::OnDBLoad(const GGlobalControlData & data)
{
	if(!_initialized)
	{
		_data = data;
		_initialized = true;
		SyncAllToGS();
	}
}

void GlobalControl::OnGSConnect(Protocol::Manager * manager, int sid)
{
	if(!_initialized) return;
	SyncAllToGS(sid);
}

/*
   元宝金币兑换错误码
		0 成功 
		1 比率超范围[500000,3000000] 
		2 比率不合法不能开启
 		3 服务器未完成初始化，稍后再试 
		-100 oper参数错误
 */
int GlobalControl::SetCashMoneyExchgRate(int rate)
{
	if(!_initialized) return 3;
	if(!IsCMERateValid(rate)) return 1;
	_data.cash_money_exchange_rate = rate;
	_data.cash_money_exchange_open = 0;

	SyncCashMoneyExchgToGS();
	SyncToDB();	
	return 0;	
}

int GlobalControl::SetCashMoneyExchgOpen(char open)
{
	if(!_initialized) return 3;
	if(open && !IsCMERateValid(_data.cash_money_exchange_rate)) return 2;
	_data.cash_money_exchange_open = open ? 1 : 0;

	SyncCashMoneyExchgToGS();
	SyncToDB();	
	return 0;	
}

void GlobalControl::SyncAllToGS(int sid)
{
	SyncCashMoneyExchgToGS(sid);
	SyncForbidListToGS(sid);
	SyncTriggerListToGS(sid);
}

void GlobalControl::SyncCashMoneyExchgToGS(int sid)
{
	CashMoneyExchangeNotify notify(_data.cash_money_exchange_open, _data.cash_money_exchange_rate); 
	if(sid == -1)
		GProviderServer::GetInstance()->BroadcastProtocol(notify);
	else
		GProviderServer::GetInstance()->Send(sid, notify);
}

void GlobalControl::SyncToDB()
{
	GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBPUTGLOBALCONTROL, DBPutGlobalControlArg(_data)));
}

int  GlobalControl::SetForbidList(Octets cmd_list)
{
	Base64Decoder base64_decoder;
	Octets decode_cmd = base64_decoder.Update(cmd_list);
	char *cmd_str = new char[decode_cmd.size()+1];
	memcpy(cmd_str,decode_cmd.begin(),decode_cmd.size());
	cmd_str[decode_cmd.size()] = '\0';
	for(unsigned int i=0; i<strlen(cmd_str); i++)
	{
		char c = cmd_str[i];
		if((c < '0' || c > '9') && c!='#' && c!=':' && c!=';')
        {
            delete [] cmd_str;
			return -1;
        }
	}
		
	char *delim = "#";
	char *outer_ptr = NULL;
	char *token = strtok_r(cmd_str,delim,&outer_ptr);
	int type = 0;
	while(NULL != token)
	{
		char *inner_delim = ":";
		char *inner_ptr = NULL;
		char *cmd_type = strtok_r(token,inner_delim,&inner_ptr);
		if(cmd_type == NULL)
		{
            delete [] cmd_str;
			return -1;
		}
		type = atoi(cmd_type);
		cmd_type = strtok_r(NULL,inner_delim,&inner_ptr);
		char *list_delim = ";";
		char *list_ptr = NULL;
		std::vector<int> list_vec;
		list_vec.push_back(atoi(cmd_type));
		cmd_type = strtok_r(NULL,inner_delim,&inner_ptr);
		char *list = strtok_r(cmd_type,list_delim,&list_ptr);
		while(list != NULL)
		{
			list_vec.push_back(atoi(list));
			list = strtok_r(NULL,list_delim,&list_ptr);
		}
		switch(type)
		{
			case FORBID_CTRL:
				MakeCMDList(_data.forbid_ctrl_list,list_vec);
				break;
			case FORBID_ITEM:
				MakeCMDList(_data.forbid_item_list,list_vec);
				break;
			case FORBID_SVR:
				MakeCMDList(_data.forbid_service_list,list_vec);
				break;
			case FORBID_TASK:
				MakeCMDList(_data.forbid_task_list,list_vec);
				break;
			case FORBID_SKILL:
				MakeCMDList(_data.forbid_skill_list,list_vec);
				break;
			case TRIGGER_CTRL:
				MakeCMDList(_data.trigger_ctrl_list,list_vec);
				break;
            case FORBID_SHOPITEM:
                MakeCMDList(_data.forbid_shopitem_list,list_vec);
				break;
            case FORBID_RECIPE:
                MakeCMDList(_data.forbid_recipe_list, list_vec);
                break;
			default:
				break;
		}
		token = strtok_r(NULL,delim,&outer_ptr);
	}
	delete [] cmd_str;
	RemoveConflictCMD();
	SyncForbidListToGS();
	SyncTriggerListToGS();
	SyncToDB();	
	return 0;
}

int GlobalControl::GetForbidList(Octets &forbid_cmd)
{
	std::ostringstream cmd_os;
	ForbidList2Stream(_data.forbid_ctrl_list,cmd_os,FORBID_CTRL);
	ForbidList2Stream(_data.forbid_item_list,cmd_os,FORBID_ITEM);
	ForbidList2Stream(_data.forbid_service_list,cmd_os,FORBID_SVR);
	ForbidList2Stream(_data.forbid_task_list,cmd_os,FORBID_TASK);
	ForbidList2Stream(_data.forbid_skill_list,cmd_os,FORBID_SKILL);
	ForbidList2Stream(_data.trigger_ctrl_list,cmd_os,TRIGGER_CTRL);
    ForbidList2Stream(_data.forbid_shopitem_list, cmd_os, FORBID_SHOPITEM);
    ForbidList2Stream(_data.forbid_recipe_list, cmd_os, FORBID_RECIPE);
	std::string cmd_str = cmd_os.str();
	Octets cmd_octets(cmd_str.c_str(),cmd_str.size());
	Base64Encoder::Convert(forbid_cmd, cmd_octets);
	return 0;
}

void GlobalControl::MakeCMDList(std::vector<int> &forbid_list,std::vector<int> &cmd_list)
{
	if(cmd_list.front() == 1)//overwrite
	{
		if(cmd_list.size() == 1)
		{
			forbid_list.clear();
		}
		else
		{
			std::vector<int> tmp(cmd_list.begin()+1,cmd_list.end());
			forbid_list.swap(tmp);
		}
	}
	else
	{
		for(unsigned int i = 1; i<cmd_list.size();i++)
		{
			forbid_list.push_back(cmd_list[i]);
		}
	}
	return;
}

void GlobalControl::ForbidList2Stream(std::vector<int> &forbid_list,std::ostringstream &cmd_os,int type)
{
	cmd_os << type <<":0:"; //查询时，均设置为非覆盖
	if(!forbid_list.empty())
	{
		for(unsigned int i=0;i<forbid_list.size();i++)
		{
			cmd_os << forbid_list[i];
			if(i!=forbid_list.size())
				cmd_os <<";";
		}
	}
	cmd_os << "#";
}

void GlobalControl::SyncForbidListToGS(int sid)
{
	ServerForbidNotify notify(_data.forbid_ctrl_list,_data.forbid_item_list,_data.forbid_service_list,_data.forbid_task_list,_data.forbid_skill_list, _data.forbid_shopitem_list, _data.forbid_recipe_list);
	if(sid == -1)
		GProviderServer::GetInstance()->BroadcastProtocol(notify);
	else
		GProviderServer::GetInstance()->Send(sid, notify);
}

void GlobalControl::SyncTriggerListToGS(int sid)
{
	ServerTriggerNotify notify(_data.trigger_ctrl_list);
	if(sid == -1)
		GProviderServer::GetInstance()->BroadcastProtocol(notify);
	else
		GProviderServer::GetInstance()->Send(sid, notify);
}

void GlobalControl::RemoveConflictCMD()
{
	std::vector<int>::iterator it = _data.trigger_ctrl_list.begin();
	for(; it!= _data.trigger_ctrl_list.end();)
	{
		if(std::find(_data.forbid_ctrl_list.begin(),_data.forbid_ctrl_list.end(),*it) != _data.forbid_ctrl_list.end())
			it = _data.trigger_ctrl_list.erase(it);
		else
			it++;
	}
}

}
