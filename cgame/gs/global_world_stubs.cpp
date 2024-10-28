#include "global_manager.h"
#include "instance/instance_manager.h"
#include "instance/battleground_manager.h"
#include "instance/countrybattle_manager.h"
#include "instance/parallel_world_manager.h"
#include "mobile/mobile_world_manager.h"
#include "instance/trickbattle_manager.h"
#include "instance/countryterritory_manager.h"
#include "instance/mnfaction_manager.h"
#include <strtok.h>
#include "start.h"

static global_world_manager *gwm = NULL;
static instance_world_manager * iwm = NULL;

namespace {
enum 
{
	INIT_ERROR = -1,
	INIT_BIG_WORLD,
	INIT_INSTANCE,
	INIT_FACTION,
	INIT_BATTLEGROUND,
	INIT_COUNTRYBATTLE,
	INIT_COUNTRYTERRITORY,
	INIT_MOBILESERVER,
	INIT_PARALLEL_WORLD,
	INIT_TRICKBATTLE,
	INIT_MNFACTIONBATTLE,
};
};


int FirstStepInit(const char * conf_file, const char * alias_file)
{
	ONET::Conf::GetInstance(conf_file);
	ONET::Conf::AppendConfFile(alias_file);
//	Conf *conf = ONET::Conf::GetInstance();
//	conf->dump(stdout);
	__PRINTINFO("配置文件:'%s'\t 配置增补文件:'%s'\n",conf_file,alias_file);
	
	//这里要读取elements.data 和tasks.data 等数据
	return world_manager::FirstStepInit();
}


int InitWorld(const char * gmconf_file, const char * servername)
{
	Conf *conf = ONET::Conf::GetInstance();
	
	//确定是世界还是副本
	int is_instance = INIT_ERROR;

	std::string servers = conf->find("General","instance_servers");
	abase::strtok tok(servers.c_str(),";,\r\n");
	const char * token;
	while((token = tok.token()))
	{       
		if(!*token) continue;
		if(strcmp(token,servername) == 0)
		{
			if(is_instance != -1)
			{
				__PRINTINFO("错误:服务器名同时存在于世界与副本列表中或重复出现多次 '%s'\n",servername);
				return -2002;
			}
			std::string str = "Instance_";
			str += servername;
			if(atoi(conf->find(str,"faction_server").c_str()) == 1)
			{
				is_instance = INIT_FACTION;
			}
			else if(atoi(conf->find(str,"battleground_server").c_str()) == 1)
			{
				is_instance = INIT_BATTLEGROUND;
			}
			else if(atoi(conf->find(str,"countrybattle_server").c_str()) == 1)
			{
				is_instance = INIT_COUNTRYBATTLE;
			}
			else if(atoi(conf->find(str,"countryterritory_server").c_str()) == 1)
			{
				is_instance = INIT_COUNTRYTERRITORY;
			}
			else if(atoi(conf->find(str,"parallelworld_server").c_str()) == 1)
			{
				is_instance = INIT_PARALLEL_WORLD;
			}
			else if(atoi(conf->find(str,"trickbattle_server").c_str()) == 1)
			{
				is_instance = INIT_TRICKBATTLE;
			}
			else if(atoi(conf->find(str,"mnfaction_server").c_str()) == 1)
			{
				is_instance = INIT_MNFACTIONBATTLE;
			}
			else
			{
				is_instance = INIT_INSTANCE;
			}
			break;
		}
	}

	servers = conf->find("General","world_servers").c_str();
	abase::strtok tok2(servers.c_str(),";,\r\n");
	while((token = tok2.token()))
	{       
		if(!*token) continue;
		if(strcmp(token,servername) == 0)
		{
			if(is_instance != -1)
			{
				__PRINTINFO("错误:服务器名同时存在于世界与副本列表中或重复出现多次 '%s'\n",servername);
				return -2001;
			}
			std::string str = "World_";
			str += servername;
			if(g_mobile_server)
			{
				is_instance = INIT_MOBILESERVER;
			}
			else
			{
				is_instance = INIT_BIG_WORLD;
			}
			break;
		}
	}


	switch (is_instance)
	{
		case INIT_BIG_WORLD:
			__PRINTINFO("开始世界逻辑初始化...\n");
			gwm = new global_world_manager();
			return gwm->Init(gmconf_file,servername);
		case INIT_INSTANCE:
			__PRINTINFO("开始副本逻辑初始化...\n");
			iwm = new instance_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_FACTION:
			__PRINTINFO("开始帮派基地初始化...\n");
			iwm = new faction_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_BATTLEGROUND:
			__PRINTINFO("开始战场初始化...\n");
			iwm = new battleground_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_COUNTRYBATTLE:
			__PRINTINFO("开始国战战场初始化\n");
			iwm = new countrybattle_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_COUNTRYTERRITORY:
			__PRINTINFO("开始国战领土地图初始化\n");
			iwm = new countryterritory_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_MOBILESERVER:
			__PRINTINFO("开始手机用户服务器初始化\n");
			gwm = new mobile_world_manager();
			return gwm->Init(gmconf_file,servername);
		case INIT_PARALLEL_WORLD:
			__PRINTINFO("开始平行世界初始化\n");
			iwm = new parallel_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_TRICKBATTLE:
			__PRINTINFO("开始战场初始化\n");
			iwm = new trickbattle_world_manager();
			return iwm->Init(gmconf_file,servername);
		case INIT_MNFACTIONBATTLE:
			__PRINTINFO("开始跨服帮战初始化\n");
			iwm = new mnfaction_world_manager();
			return iwm->Init(gmconf_file,servername);
		default:
			__PRINTINFO("错误：在配置文件中没有找到正确的世界'%s'\n",servername);
			return -2003;
	}
}

