#include <ASSERT.h>
#include <strtok.h>
#include <strings.h>
#include <conf.h>

#include "world.h"
#include "common/message.h"
#include "worldmanager.h"
#include "matter.h"
#include "item.h"
#include "task/taskman.h"
#include "player_imp.h"
#include "npcgenerator.h"
#include "template/globaldataman.h"
#include "petdataman.h"
#include "ai/policy_loader.h"
#include "global_drop.h"
#include <gsp_if.h>
#include "antiwallow.h"
#include "wallow/wallowobj.h"
#include <meminfo.h>
#include "public_quest.h"
#include <db_if.h>
#include "global_controller.h"
#include "titlemanager.h"
#include "item/set_addon.h"
#include "item/generalcard_set_man.h"
#include "start.h"
#include "skillwrapper.h"
#include "template/el_region.h"

MapResManager	world_manager::_mapres;
itemdataman 	world_manager::_dataman;
path_manager 	world_manager::_pathman;
int 		world_manager::_world_index = 0;
XID 		world_manager::_server_xid(GM_TYPE_SERVER,0);
world_manager * world_manager::_manager_instance = 0;
size_t 		world_manager::_player_max_count = GL_MAX_PLAYER_COUNT;
size_t 		world_manager::_npc_max_count = GL_MAX_NPC_COUNT;
size_t 		world_manager::_matter_max_count = GL_MAX_MATTER_COUNT;
int		world_manager::_world_tag = -1;
abase::vector<int> world_manager::_instance_tag_list;
world_limit 	world_manager::_world_limit;
int 		world_manager::_region_file_tag;
int 		world_manager::_precinct_file_tag;
world_pos 	world_manager::_save_point = { -1,A3DVECTOR(0.f,0.f,0.f)};
world_pos 	world_manager::_kickout_point = { -1,A3DVECTOR(0.f,0.f,0.f)};
float		world_manager::_max_mob_sight_range = 15.f;
rest_time_ctrl 	world_manager::_rest_ctrl;
world_param	world_manager::_world_param = { false, false, false, false, false, false};
ai_trigger::manager world_manager::_trigger_man;
int 		world_manager::_npc_idle_heartbeat = NPC_IDLE_HEARTBEAT;
int 		world_manager::_lua_data_version = 0;
netgame::mall 	world_manager::_player_mall;
netgame::mall 	world_manager::_player_mall2;
netgame::touchshop world_manager::_touch_shop;
abase::hash_map<int, int> world_manager::_expire_items;
abase::hash_map<int, int> world_manager::_noputin_usertrash_items;
float		world_manager::_height_limit = 0.f;
abase::hash_map<int, int> world_manager::_region_collision_table;
float 		world_manager::_double_exp_factor = 1.f;
public_quest_manager world_manager::_public_quest_man;
ForceGlobalDataMan world_manager::_force_global_data_man;
std::set<int> world_manager::_consumption_items_shopping; // 购买时计入消费值的物品列表
abase::hash_map<int, int> world_manager::_consumption_items_binding; // 天人合一时计入消费值的物品列表
abase::hash_map<int, int> world_manager::_consumption_items_destroying; // 消耗时计入消费值的物品列表
unsigned int			world_manager::_world_team_uid = 0;
int 		world_manager::_world_team_uid_lock = 0;
GlobalController world_manager::_global_controller;
abase::hash_map<int, A3DVECTOR> world_manager::_central_server_birth_pos_map;
title_manager world_manager::_title_man;
history_manager world_manager::_history_man;
UniqueDataClient world_manager::_unique_man;
autoteam_man world_manager::_autoteam_man;
fatering_manager world_manager::_fatering_man;
world_config world_manager::_world_config;
world_flags world_manager::_world_flags = {false,false};
abase::hash_map< int, abase::vector<int> > world_manager::_region_waypoint_map;

bool 
world_manager::InitTagList()
{
	Conf *conf = Conf::GetInstance();
	Conf::section_type section = "Instance_";
	std::string servers = conf->find("General","instance_servers").c_str();
	abase::strtok tok(servers.c_str(),";,\r\n");
	const char * token;
	while((token = tok.token()))
	{       
		if(!*token) continue;
		std::string osvr = section + token;
		int tag = atoi(conf->find(osvr,"tag").c_str());
		if(tag <= 0) return false;
		for(size_t i = 0; i < _instance_tag_list.size(); i ++)
		{
			if(_instance_tag_list[i] == tag)
			{
				return false;
			}
		}
		_instance_tag_list.push_back(tag);
	}
	return true;
}

bool 
world_manager::InitWorldLimit(const char * servername)
{
	memset(&_world_limit,0,sizeof(_world_limit));
	Conf *conf = Conf::GetInstance();
	_height_limit = atof(conf->find(servername, "height_limit").c_str());
	std::string limit = conf->find(servername ,"limit");
	abase::strtok tok(limit.c_str(),";,\r\n");
	const char * token;
	while((token = tok.token()))
	{       
		if(!*token) continue;
		if(strcmp(token,"nofly") == 0)
		{
			_world_limit.nofly = true;
		}
		else if(strcmp(token,"nothrow") == 0)
		{
			_world_limit.nothrow = true;
		}
		else if(strcmp(token,"clear-ap") == 0)
		{
			_world_limit.clearap = true;
		}
		else if(strcmp(token,"allow-root") == 0)
		{
			_world_limit.allowroot = true;
		}
		else if(strcmp(token,"use-save-point") == 0)
		{
			_world_limit.savepoint = true;
		}
		else if(strcmp(token,"nomount") == 0)
		{
			_world_limit.nomount = true;
		}
		else if(strcmp(token,"gm-free") == 0)
		{
			_world_limit.gmfree = true;
		}
		else if(strcmp(token,"noduel") == 0)
		{
			_world_limit.noduel = true;
		}
		else if(strcmp(token,"no-bind") ==0)
		{
			_world_limit.nobind = true;
		}
		else if(strcmp(token,"ctrlid-open-instance") == 0)
		{
			_world_limit.ctrlid_open_instance = true;
		}
		else if(strcmp(token,"anti-cheat") == 0)
		{
			_world_limit.anti_cheat = true;
		}
		else if(strcmp(token,"no-market") == 0)
		{
			_world_limit.no_market = true;
		}
		else if(strcmp(token,"height-limit") == 0)
		{
			_world_limit.height_limit = true;
		}
		else if(strcmp(token,"commondata") == 0)
		{
			_world_limit.common_data_notify = true;
		}
		else if(strcmp(token,"lowjump") == 0)
		{
			_world_limit.lowjump = true;	
		}
		else if(strcmp(token,"no-multi-exp") == 0)
		{
			_world_limit.no_multi_exp = true;	
		}
		else if (strcmp(token,"profit-time") == 0)
		{
			_world_limit.profit_time = true;
		}
		else if (strcmp(token,"profit-time2") == 0)
		{
			_world_limit.profit_time2 = true;
		}
		else if(strcmp(token,"online-award") == 0)
		{
			_world_limit.online_award = true;	
		}
		else if(strcmp(token,"nothrow-anyway") == 0)
		{
			_world_limit.nothrow_anyway = true;
		}
		else if(strcmp(token,"can-reenter") == 0)
		{
			_world_limit.can_reenter = true;
		}
		else if(strcmp(token,"noauto-resurrect") == 0)
		{
			_world_limit.noauto_resurrect = true;
		}
	}

	return true;
}

bool world_manager::InitExpireItems()
{
	Conf *conf = Conf::GetInstance();
	std::string limit = conf->find("General" ,"expire_item");
	abase::strtok tok(limit.c_str(),";,|/\\\r\n");
	const char * token;
	while((token = tok.token()))
	{       
		if(!*token) continue;
		if(strchr(token,':') == NULL)
		{
			int item = atoi(token);
			if(item > 0)
			{
				_expire_items[item] = 0;
				__PRINTINFO("物品%d为时效物品\n", item);
			}
		}
		else
		{
			int item, year, month, day;
			if(sscanf(token, "%d:%d-%d-%d", &item, &year, &month, &day) == 4)
			{
				struct tm tm1;
				memset(&tm1,0,sizeof(tm1));
				tm1.tm_sec = 0;
				tm1.tm_min = 0;
				tm1.tm_hour = 0;
				tm1.tm_mday = day;
				tm1.tm_mon = month - 1;
				tm1.tm_year = year - 1900;
				int t;
				if( (t = mktime(&tm1)) != -1)
				{
					_expire_items[item] = t;		
					__PRINTINFO("物品%d为时效物品,过期时间%d\n", item, t);
				}
			}		
		}
	}
	return true;
}

bool world_manager::IsExpireItem(int id) 
{
	abase::hash_map<int, int>::iterator it = _expire_items.find(id);
	return it != _expire_items.end() && (it->second == 0 || g_timer.get_systime() >= it->second);
}

bool world_manager::IsMallConsumptionItemShopping(int id)
{
	return _consumption_items_shopping.find(id) != _consumption_items_shopping.end();
}

bool world_manager::GetMallConsumptionValueBinding(int id, int& value)
{
	abase::hash_map<int, int>::iterator it;
	it = _consumption_items_binding.find(id);
	if (it == _consumption_items_binding.end())
	{
		return false;
	}
	else
	{
		value = it->second;
		return true;
	}
}

bool world_manager::GetMallConsumptionValueDestroying(int id, int& value)
{
	abase::hash_map<int, int>::iterator it;
	it = _consumption_items_destroying.find(id);
	if (it == _consumption_items_destroying.end())
	{
		return false;
	}
	else
	{
		value = it->second;
		return true;
	}
}

bool world_manager::InitNoPutInUserTrashItems()
{
	//应该在加载物品模板文件后进行
	DATA_TYPE dt;
	ACC_STORAGE_BLACKLIST_CONFIG * cfg = (ACC_STORAGE_BLACKLIST_CONFIG *)GetDataMan().get_data_ptr(696, ID_SPACE_CONFIG, dt);
	if(!cfg || dt != DT_ACC_STORAGE_BLACKLIST_CONFIG) return false;
	for(size_t i=0; i < sizeof(cfg->blacklist)/sizeof(unsigned int); i++)
	{
		if(cfg->blacklist[i] > 0) _noputin_usertrash_items[cfg->blacklist[i]] = 1;  	
	}
	return true;
}

static bool GetWallowConf(const char * name, anti_wallow::punitive_param  &p)
{
	Conf *conf = Conf::GetInstance();
	p.exp 		= atof(conf->find(name,"exp").c_str());
	p.sp		= atof(conf->find(name,"sp").c_str());
	p.item		= atof(conf->find(name,"item").c_str());
	p.money		= atof(conf->find(name,"money").c_str());
	p.task_exp	= atof(conf->find(name,"task_exp").c_str());
	p.task_sp	= atof(conf->find(name,"task_sp").c_str());
	p.task_money	= atof(conf->find(name,"task_money").c_str());
	return *conf->find(name,"exp").c_str();
}
	
bool world_manager::InitWallowParam()
{
	anti_wallow::punitive_param p = {true, 1.0,1.0,1.0,1.0,1.0,1.0,1.0};
	anti_wallow::SetParam(0, p);
	anti_wallow::SetParam(1, p);

	if(!GetWallowConf("WallowLight", p)) return false;
	anti_wallow::SetParam(2, p);

	if(!GetWallowConf("WallowHeavy", p)) return false;
	anti_wallow::SetParam(3, p);

	Conf *conf = Conf::GetInstance();
	
	int light,heavy,clear;
	light = atoi(conf->find("WallowTime", "TimeLight").c_str());
	heavy = atoi(conf->find("WallowTime", "TimeHeavy").c_str());
	clear = atoi(conf->find("WallowTime", "TimeClear").c_str());
	bool mode = strcmp(conf->find("WallowTime", "ClearMode").c_str(), "TimeOfDay") == 0;
	__PRINTINFO("沉迷的三个时间参数为%d,%d,%d ,沉迷恢复方式为:'%s'\n",light,heavy, clear,mode?"每天零点":"休息时间到达");
	wallow_object::SetParam(light,heavy,clear,mode?1:0);

	std::string str = conf->find("General", "AntiWallow");
	if(strcmp(str.c_str() , "true") == 0)
	{
		_world_param.anti_wallow = 1;
		__PRINTINFO("防沉溺功能系统开启\n");
	}
	else if(strcmp(str.c_str(), "false") == 0)
	{
		_world_param.anti_wallow = 0;
		__PRINTINFO("防沉溺功能系统关闭\n");
	}
	else
	{
		__PRINTINFO("未找到防沉迷开关状态 默认关闭\n");
	}

	//这里判断反外挂
	str = conf->find("General", "AntiCheat");
	if(strcmp(str.c_str() , "true") == 0)
	{
		_world_param.anti_cheat = 1;
		__PRINTINFO("反外挂惩罚功能开启\n");
	}
	else if(strcmp(str.c_str(), "false") == 0)
	{
		_world_param.anti_cheat = 0;
		__PRINTINFO("反外挂惩罚功能关闭\n");
	}
	else
	{
		__PRINTINFO("未找到反外挂开关状态 默认关闭\n");
	}

	_world_param.korea_shop = (strcmp(conf->find("General", "KoreaVersion").c_str() , "true") == 0);
	if(_world_param.korea_shop)
	{
		printf("使用朝鲜语版本内容\n");
	}
	else
	{
		printf("使用标准版本内容\n");
	}
	_world_param.southamerican_shop = (strcmp(conf->find("General","SouthAmericanVersion").c_str(),"true") == 0);
	if(_world_param.southamerican_shop)
	{
		printf("使用南美版本内容\n");
	}
	else
	{
		printf("使用标准版本内容\n");
	}
	ASSERT(!(_world_param.korea_shop && _world_param.southamerican_shop));
	
	_world_param.random_shop_limit = (strcmp(conf->find("General","RandomShopLimit").c_str(),"true") == 0);
	if(_world_param.random_shop_limit)
	{
		printf("关闭随机商城\n");
	}
	else
	{
		printf("开启随机商城\n");
	}

	return true;
}

bool world_manager::InitCentralServerBirthPos()
{
	Conf *conf = Conf::GetInstance();

	std::string accepted_zones = conf->find("General", "accepted_zone_list");
	std::string cross_zone_pos = conf->find("General", "cross_zone_pos");

	abase::strtok zone_tok(accepted_zones.c_str(),";,");
	abase::strtok pos_tok(cross_zone_pos.c_str(),";,");
	const char * zone_token; 
	const char * pos_token;
	while((zone_token = zone_tok.token()) && (pos_token = pos_tok.token()))
	{
		if(!*zone_token) break;
		int zoneid = atoi(zone_token);
		if(zoneid <= 0) break;
		if(!*pos_token) break;
		A3DVECTOR pos;
		int n = sscanf(pos_token, "%f:%f:%f", &pos.x, &pos.y, &pos.z);
		if(n != 3) break;
		_central_server_birth_pos_map[zoneid] = pos;
	}
	return true;
}

bool world_manager::InitWorldConfig()
{
	DATA_TYPE dt;
	PROFIT_TIME_CONFIG * ptcfg = (PROFIT_TIME_CONFIG *)GetDataMan().get_data_ptr(1735, ID_SPACE_CONFIG, dt);
	if(!ptcfg || dt != DT_PROFIT_TIME_CONFIG) return false;
	
	_world_config.profit_time = ptcfg->upper_limit * 3600;
	if(_world_config.profit_time <= 0 || _world_config.profit_time >= 24*3600)
	{
		__PRINTF("收益时间配置参数不正确. profit_time = %d", _world_config.profit_time);
		return false;
	}
	
	return true;
}

void world_manager::SetWorldIndex(int world_index) 
{
	ASSERT(world_index >= 0);
	_world_index =  world_index;
	_server_xid.id = world_index;
}

int 
world_message_handler::HandleMessage(world * pPlane,const MSG& msg)
{
	switch(msg.message)
	{
		case GM_MSG_PRODUCE_MONEY:
			{
				ASSERT(msg.content_length == sizeof(msg_gen_money));
				msg_gen_money * mgi = (msg_gen_money*)msg.content;
				A3DVECTOR pos(msg.pos);
				pos.x += abase::RandUniform() * 4.f - 2.f;
				pos.z += abase::RandUniform() * 4.f - 2.f;
				const grid * pGrid = &pPlane->GetGrid();
				if(!pGrid->IsLocal(pos.x,pos.z))
				{
					pos.x = msg.pos.x;
					pos.z = msg.pos.z;
				}
				float h = pPlane->GetHeightAt(pos.x,pos.z);
				if(pos.y < h) pos.y = h;
				DropMoneyItem(pPlane,pos,msg.param,msg.source,mgi->team_id,mgi->team_seq);
			}
			return 0;

		case GM_MSG_PRODUCE_MONSTER_DROP:
			{
				ASSERT(msg.content_length > 3 * sizeof(int));
				A3DVECTOR pos(msg.pos);
				const int * list = (const int*)msg.content;
				ASSERT(list[3]*sizeof(int) + sizeof(int)*4 == msg.content_length);
				// list[0] team_id
				// list[1] team_seq
				// list[2] npc_id
				// list[3] count
				// msg.param money 
				const grid * pGrid = &pPlane->GetGrid();
				if(msg.param > 0)
				{
					pos.x = msg.pos.x + abase::RandUniform() * 4.f - 2.f;
					pos.z = msg.pos.z + abase::RandUniform() * 4.f - 2.f;
					if(!pGrid->IsLocal(pos.x,pos.z))
					{
						pos.x = msg.pos.x;
						pos.z = msg.pos.z;
					}
					float h = pPlane->GetHeightAt(pos.x,pos.z);
					if(pos.y < h) pos.y = h;
					DropMoneyItem(pPlane,pos,msg.param,msg.source,list[0],list[1]);
				}       

				struct 
				{
					char type;
					char size;
					short name[10];
				}tag;
				tag.type = element_data::IMT_DROP;
				tag.size = 0;
				/*DATA_TYPE dt;
				MONSTER_ESSENCE * pess = (MONSTER_ESSENCE*)world_manager::GetDataMan().get_data_ptr(list[2], ID_SPACE_ESSENCE, dt);
				if(pess && dt == DT_MONSTER_ESSENCE)
				{
					size_t o = 0;
					for(; o < 10; o ++)
					{
						if(pess->name[o] == 0) break;
						tag.name[o] = pess->name[o];
					}
					tag.size = o * 2;
				}*/
				

				for(int i = 0; i < list[3]; i ++)
				{
					//item_data * data = world_manager::GetDataMan().generate_item_for_drop(list[4+i],&tag,sizeof(tag));
					item_data * data = world_manager::GetDataMan().generate_item_for_drop(list[4+i],&tag,tag.size + 2);
					if(data)
					{
						pos.x = msg.pos.x + abase::RandUniform() * 4.f - 2.f;
						pos.z = msg.pos.z + abase::RandUniform() * 4.f - 2.f;
						if(!pGrid->IsLocal(pos.x,pos.z))
						{
							pos.x = msg.pos.x;
							pos.z = msg.pos.z;
						}
						float h = pPlane->GetHeightAt(pos.x,pos.z);
						if(pos.y < h) pos.y = h;
						DropItemData(pPlane,pos,data,msg.source,list[0],list[1],0);
						world_manager::TestCashItemGenerated(data->type, data->count);
					}       
				}       
			}       
			return 0;

		case GM_MSG_PLANE_SWITCH_REQUEST:
		//确定切换请求 
		//直接返回反馈 可能需要检查是否可以传送等
		{
			MSG nmsg = msg;
			nmsg.target = msg.source;
			nmsg.source = msg.target;
			nmsg.message = GM_MSG_PLANE_SWITCH_REPLY;
			pPlane->PostLazyMessage(nmsg);
		};
			return 0;

		default:
			__PRINTINFO("未处理的世界消息%d\n",msg.message);
	}
	return 0;
}

/*bool 
world_manager::InitTerrain(std::string base_path, std::string section,const rect & rt)
{
	Conf *conf = Conf::GetInstance();

	TERRAINCONFIG config;
	config.nAreaWidth = atoi(conf->find(section,"nAreaWidth").c_str());
	config.nAreaHeight = atoi(conf->find(section,"nAreaHeight").c_str());
	config.nNumAreas = atoi(conf->find(section,"nNumAreas").c_str());
	config.nNumCols = atoi(conf->find(section,"nNumCols").c_str());
	config.nNumRows =atoi(conf->find(section,"nNumRows").c_str());
	config.vGridSize = atof(conf->find(section,"vGridSize").c_str());
	config.vHeightMin = atof(conf->find(section,"vHeightMin").c_str());
	config.vHeightMax = atof(conf->find(section,"vHeightMax").c_str());
	memset(config.szMapPath,0,sizeof(config.szMapPath));
	std::string path = base_path + conf->find(section,"szMapPath");
	strncpy(config.szMapPath, path.c_str(),sizeof(config.szMapPath) - 1);

	ASSERT(rt.left <= rt.right && rt.top <= rt.bottom);
	if(!_terrain.Init(config,rt.left,rt.top,rt.right,rt.bottom))
	{
		__PRINTINFO("无法初始化地形数据\n");
		return false;
	}
	__PRINTINFO("terrain data loaded\n");

	std::string trace_file = conf->find("Template","CollisionFile");
	if(strlen(trace_file.c_str()) && ( GetMemTotal() >3*1024*1024 || player_template::GetDebugMode()))      //3G 一下内存不启动本碰撞检测
	{
		std::string trace_path = base_path + trace_file;
		if(!_trace_man.Load(trace_path.c_str()))
		{
			printf("加载凸包数据'%s'失败\n",trace_path.c_str());
		}
		else
		{

			printf("加载'%s'完成\n",trace_path.c_str());
		}
	}
	return true;
}*/

bool 
world_manager::InitQuestSystem(const char * path, const char * path2)
{
	return ::InitQuestSystem(path,path2,world_manager::GetDataMan().GetElementDataMan());
}


int 
world_manager::PlaneSwitch(gplayer_imp * pImp,const A3DVECTOR & pos,int tag,const instance_key & key, size_t fee)
{
	gplayer * pPlayer = (gplayer*)pImp->_parent;
	ASSERT(pPlayer->spinlock);
	if(tag == _world_tag && pImp->_plane->PosInWorld(pos))
	{	
		// 是自己
		if(fee)
		{
			pImp->SpendMoney(fee);
			pImp->_runner->spend_money(fee);
		}
		pImp->LongJump(pos);
		return 0;
	}

	if(tag == 121 || tag == 122)	//仙魔界对人物修真的限制
	{
		if(pImp->_basic.sec_level < 20)
			return -1;
	}

	int index = _ioman.GetGlobalServer(pos,tag);
	if(index < 0)
	{
		return -1;
	}
	ASSERT(index != GetWorldIndex());

	//进行是否可以进行副本重置的检测
	int ins_timer = pImp->CheckInstanceTimer(tag);
/*	{
		pImp->_runner->error_message(S2C::ERR_CAN_NOT_RESET_INSTANCE);
		//不会再次报错所以返回0
		return 0;
	}*/
	
	//减少金钱
	if(fee)
	{
		pImp->SpendMoney(fee);
		pImp->_runner->spend_money(fee);
	}
	instance_key key2 = key;

	MSG msg;
	BuildMessage(msg,GM_MSG_PLANE_SWITCH_REQUEST,XID(GM_TYPE_SERVER,index),pImp->_parent->ID,pos,
			ins_timer,&key2,sizeof(key2));
	SendRemoteMessage(index,msg);
	//pImp->_commander->SwitchSvr(index,pPlayer->pos,pos,instance_key);
	return 0;
}

bool 
world_manager::InitNPCTemplate()
{
	//手机用户服务器不加载npc模板数据
	if(!g_mobile_server && !npc_stubs_manager::LoadTemplate(GetDataMan()))
	{
		return false;
	}

	__PRINTINFO("Before recipe_manager::loadtemplate\n");
	if(!recipe_manager::LoadTemplate(GetDataMan()))
	{
		__PRINTINFO("recipe_manager::loadtemplate .... FAILED\n");
		return false;
	}
	__PRINTINFO("After recipe_manager::loadtemplate\n");

	if(!pet_dataman::LoadTemplate(GetDataMan()))
	{
		return false;
	}
	return true;
}

namespace 
{
	template <typename T>
	bool VeriyClass(int guid)
	{
		ClassInfo * pInfo;
		pInfo = ClassInfo::GetRunTimeClass(guid);
		return pInfo && pInfo->IsDerivedFrom(T::GetClass());
	}
}

bool 
world_manager::player_cid::Init(const char * str)
{
	abase::strtok tok(str, ";, \r\n");
	int index = 0;
	const char * ch;
	while( (ch = tok.token()))
	{
		cid[index] = atoi(ch);
		index ++;
		if(index == 3) break;
	}
	if(index != 3) return false;

	if(!VeriyClass<gplayer_imp>(cid[0])) return false;
	if(!VeriyClass<gplayer_controller>(cid[1])) return false;
	if(!VeriyClass<gplayer_dispatcher>(cid[2])) return false;
	return true;
}

void
world_manager::ShutDown()
{
	gplayer * pPool = GetPlayerPool();
	for(size_t i = 0; i<world_manager::GetMaxPlayerCount(); i ++)
	{
		if(pPool[i].IsEmpty()) continue;
		if(!pPool[i].imp) continue;
		spin_autolock keeper(pPool[i].spinlock);
		if(!pPool[i].IsActived()) continue;
		if(!pPool[i].imp) continue;
		if(pPool[i].login_state != gplayer::LOGIN_OK) continue;
		pPool[i].imp->ServerShutDown();
	}
	if(_serverdata_init)
		ShutDownSaveServerData();
	sleep(15);
	kill(getpid(),15);
}

void
world_manager::Init()
{
	w_npc_man.Init(world_manager::GetMaxNPCCount());
	w_matter_man.Init(world_manager::GetMaxMatterCount());
	w_player_man.Init(world_manager::GetMaxPlayerCount());
}

void 
world_manager::Heartbeat()
{
	abase::RandUniform();
	w_npc_man.OnHeartbeat();
	w_matter_man.OnHeartbeat();
	w_player_man.OnHeartbeat();

	_unique_man.OnHeartbeat();

	if(_serverdata_init)
	{
		if(--_write_timer <= 0)
		{
			AutoSaveServerData();
			_write_timer = abase::Rand(9000,11000);		
		}
	}
}

int 
world_manager::FirstStepInit()
{
	Conf *conf = Conf::GetInstance();
	std::string root = conf->find("Template","Root");

	//初始化附加属性
	InitAllAddon();

	//这里进行策略装载
	std::string policy_path = root + conf->find("Template","PolicyData");
	bool bRst = LoadAIPolicy(policy_path.c_str());
	if(!bRst)
	{
		__PRINTINFO("装载策略文件失败:'%s'\n",policy_path.c_str());
		return -300;
	}

	if(!GNET::SkillWrapper::Initialize())
	{
		__PRINTINFO("初始化技能失败");
		return -301;
	}

	std::string path = root + conf->find("Template","itemDataFile");

	bool disable_bind2 = (conf->find("General","DisableBind2") == "true");
	if(_dataman.load_data(path.c_str(), disable_bind2))
	{
		__PRINTINFO("无法读取物品和怪物的模板文件:'%s'\n",path.c_str());
		return -3;
	}

	if(!disable_bind2)
	{
		__PRINTINFO("装备后绑定功能打开\n");
	}

	if(!set_addon_manager::LoadTemplate(_dataman))
	{
		__PRINTINFO("初始化套装属性和天书属性和卡牌属性失败\n");
		return -7;
	}

	if(!generalcard_set_manager::LoadTemplate(_dataman))
	{
		__PRINTINFO("初始化卡牌套装失败\n");
		return -8;
	}

	std::string path1 = root + conf->find("Template","QuestPackage");
	std::string path2 = root + conf->find("Template","QuestPackage2");
	if(!InitQuestSystem(path1.c_str(),path2.c_str()))
	{
		__PRINTINFO("读取任务系统失败:'%s','%s'\n",path1.c_str(), path2.c_str());
		return -4;
	}

	path = root + conf->find("Template","DropData");
	if(load_extra_drop_tables(path.c_str()))
	{
		__PRINTINFO("读取全局掉落表失败:'%s'\n",path.c_str());
		return -5;
	}

	if(!drop_template::LoadDropList())
	{
		__PRINTINFO("初始化全局掉落表失败\n");
		return -6;
	}

	//读取全局数据和百宝阁数据
	path1 = root + conf->find("Template","GlobalData");
	path2 = root + conf->find("Template","MallData");
	std::string path3 = conf->find("Template","Mall2Data");
	if(path3.length()!=0) path3 = root + path3;
	if(!globaldata_loadserver(path1.c_str(),path2.c_str(),path3.c_str()))
	{
		__PRINTINFO("读取全局数据或读取百宝阁数据或读取鸿利商城失败:'%s' '%s' '%s'\n",path1.c_str(),path2.c_str(),path3.c_str());
		return -18;
	}
	
	//初始化百宝阁
	if(!netgame::InitMall(_player_mall, _dataman, globaldata_getmallitemservice()))
	{
		__PRINTINFO("初始化百宝阁数据失败\n");
		return -31;
	}
	
	//初始化鸿利商城
	if(!netgame::InitMall(_player_mall2, _dataman, globaldata_getmall2itemservice()))
	{
		__PRINTINFO("初始化鸿利商城数据失败\n");
		return -32;
	}

	//touch shop 
	if(!netgame::InitTouchShop(_touch_shop, _dataman))
	{
		__PRINTINFO("初始化touch shop数据失败\n");
		return -30;
	}

	if(!GetTitleMan().InitTitle(_dataman))
	{
		__PRINTINFO("初始化title数据失败\n");
		return -29;
	}

	if(!GetFateRingMan().Initialize(_dataman))
	{
		__PRINTINFO("初始化命轮失败\n");
		return -35;
	}

	if(!GetAutoTeamMan().InitAutoTeamConfig(_dataman))
	{
		__PRINTINFO("初始化autoteam数据失败\n");
		return -36;
	}

	if(!InitNPCTemplate())
	{
		__PRINTINFO("初始化配方和NPC失败\n");
		return -17;
	}

	//读取RareItem列表
	path = root + conf->find("Template","RareItem");
	LoadRareItemList(path.c_str());

	//讀取腳本文件版本
	path = root + conf->find("Template","LuaData");
	if(!LoadLuaVersion(path.c_str()))
	{
		__PRINTINFO("无法读取LUA脚本文件'%s'\n", path.c_str());
		return -18;
	}

	//装载NPC MATTER DYN_OBJECT的凸包数据
	//手机用户服务器不加载凸包
	path = root + conf->find("Template","CollisionElement");
	if(!g_mobile_server && !trace_manager2::LoadElement(path.c_str()))
	{
		printf("加载元素凸包数据据失败'%s'",path.c_str());
		return - 19;
	}

	if(!LoadMallConsumptionConfig())
	{
		printf("加载消费值配置表失败");
		return -33;
	}
	
	//初始化屏蔽字符
	path = root + conf->find("Template","table_name");
	if (!GMSV::InitMatcher(path.c_str(), "UCS2", 
				conf->find("Template","name_charset").c_str(), conf->find("Template","table_charset").c_str()))
	{
		__PRINTF("初始化屏蔽字符失败\n");
		return -34;
	}
	
	return 0;
}

int
world_manager::InitBase(const char * section)
{
	world_manager::InitExpireItems();
	world_manager::InitNoPutInUserTrashItems();
	world_manager::InitWorldLimit(section);
	if(!world_manager::InitWallowParam())
	{
		__PRINTINFO("读取防沉迷数据失败\n");
		return -122;
	}

	if(!world_manager::InitTagList())
	{
		__PRINTINFO("读取副本标识失败\n");
		return -123;
	}

	if(!_world_limit.allowroot && (getuid() == 0 || geteuid()==0))
	{
		__PRINTINFO("目前的设置不允许root启动程序\n");
		return -124;
	}

	Conf *conf = Conf::GetInstance();

	std::string root = conf->find("Template","Root");

	//取得可能存在的存盘点
	std::string str = conf->find(section,"save_point");
	{
		int tag = -1;
		A3DVECTOR pos;
		sscanf(str.c_str(),"%d,%f,%f,%f",&tag,&pos.x,&pos.y,&pos.z);
		if(tag <=0) tag = -1;
		_save_point.tag = tag;
		_save_point.pos = pos;
	}
	
	//取得可能存在的踢出点
	str = conf->find(section,"kickout_point");
	{
		int tag = -1;
		A3DVECTOR pos;
		sscanf(str.c_str(),"%d,%f,%f,%f",&tag,&pos.x,&pos.y,&pos.z);
		if(tag <=0) tag = -1;
		_kickout_point.tag = tag;
		_kickout_point.pos = pos;
	}

	time_t ct = time(NULL);
	__PRINTINFO("%s\n",ctime(&ct));

	//设置世界索引和世界标记
	SetWorldIndex(atoi(conf->find(section,"index").c_str()));
	SetWorldTag(atoi(conf->find(section,"tag").c_str()));

	//读取最大视野距离
	float mmsr = atof(conf->find(section,"max_sight_range").c_str());
	if(mmsr < 1.0f) mmsr = DEFAULT_AGGRESSIVE_RANGE;
	_max_mob_sight_range = mmsr;

	__PRINTINFO("Starting world No.%d\n",_world_index);
	if(_world_tag < 0 || _world_index < 0)
	{
		__PRINTINFO("错误的world index或 world tag\n");
		return -1009;
	}


	//读取打卡时间参数
	str = conf->find("DoubleExp","clear_day");
	int min,hours,mday;
	sscanf(str.c_str(),"%d %d %d",&min,&hours,&mday);
	if(min < 0 || min >= 60 || hours <0 || hours >= 24 || mday < 0 || mday>=7) 
	{
		__PRINTINFO("双倍打卡时间起始时间不正确\n");
		return -18;
	}
	char *weeks[] ={"日","一","二","三","四","五","六"};
	__PRINTINFO("双倍经验时间每周起始时间为:星期%s %02d:%02d\n",weeks[mday],hours,min);

	int first_rest_time = atoi(conf->find("DoubleExp","first_rest_time").c_str());
	int rest_period = atoi(conf->find("DoubleExp","rest_period").c_str());
	int rest_time_unit = atoi(conf->find("DoubleExp","rest_time_unit").c_str());
	int max_rest_time  = atoi(conf->find("DoubleExp","max_rest_time").c_str());

	if(first_rest_time < 0 || rest_period <= 0 || rest_time_unit < 0 || max_rest_time <= 0 || max_rest_time > 3600*200)
	{
		__PRINTINFO("双倍时间参数不正确 \n");
		return -19;
	}
	rest_time_ctrl ctrl = {{min,hours,mday},first_rest_time,rest_period*3600,rest_time_unit,max_rest_time};
	_rest_ctrl = ctrl;

	//读取全局控制器
	//std::string gcontroller_path = root + conf->find("Template","GlobalController");;
	std::string gcontroller_path = root + "globalcontroller.conf";
	if(!_global_controller.Init(gcontroller_path.c_str()))
	{
		__PRINTINFO("初始化全局控制器失败\n");
		return -20;	
	}

	_unique_man.SetWorldTag(GetWorldTag()); //  需要在SetWorldTag之后

	if(!GetHistoryMan().Initialize(_dataman))
	{
		__PRINTINFO("初始化history数据失败\n");
		return -21;
	}

	//读取中央服务器上玩家的出生坐标
	world_manager::InitCentralServerBirthPos();

	//读取来自element的配置信息，要在worldtag和element初始化后进行
	if(!world_manager::InitWorldConfig())
	{
		__PRINTINFO("初始化世界配置出错\n");
		return -22;
	}
	
	return 0;
}


void 
world_manager::CalcRestTime(int & last_time, int & rest_pool_cur, int &rest_pool_cap, int mafia_id, int & mafia_rest_cap, int & mafia_last_time)
{
	time_t t = g_timer.get_systime();
	if(((int)(t - last_time) >= 7 * 24 * 3600))
	{
		//和上次计时比,超过了一周,所以进行时间调整
		struct tm tt;
		localtime_r(&t, &tt);
		int offset_m = tt.tm_min - _rest_ctrl.clear_time.min;
		int offset_h = tt.tm_hour - _rest_ctrl.clear_time.hour;
		int offset_d = tt.tm_wday - _rest_ctrl.clear_time.day;

		int offset_sec = offset_d*24*3600 + offset_h * 3600 + offset_m*60;
		time_t xt = t- offset_sec - tt.tm_sec;
	
		if(offset_sec < 0)
		{
			//表示现在时间未超过更新计时点,所以需要倒退到上一周作为计时点
			last_time = xt - 7*24*3600;
		}
		else
		{
			//表示现在时间超过了本周更新计时点
			//用新的计时点作为数值
			last_time = xt;
		}
		
		//同时需要清空当前的时间池积累时间
		rest_pool_cur = 0;

	}
	else
	{
		//不到一周,则进行通常处理
		//这里不需要做任何操作
	}
	if(t < last_time) return;			//时间调整会引发此种情况，不进行任何计算即可

	//进行帮派处理的计算
	if(mafia_last_time < last_time)
	{
		//只有上个帮派处理的时间点小于当前的时间点，才能加入帮派奖励
		mafia_rest_cap = 0;
		if(mafia_id)
		{
			//如果是帮派成员，才进行帮派时间加入操作
			int value = GMSV::GetFactionBonus(mafia_id);
			if(value > 0)
			{
				mafia_rest_cap = value * 3600;
			}
		}
		mafia_last_time = t;
	}

	//这里计算目前应该有的最大双倍时间积累值
	
	int pool_cap = _rest_ctrl. first_rest_time;
	pool_cap = pool_cap + ((t - last_time)/_rest_ctrl.rest_period) * (_rest_ctrl.rest_time_unit);
	if(pool_cap > _rest_ctrl.max_rest_time) pool_cap = _rest_ctrl.max_rest_time;
	rest_pool_cap = pool_cap;

}

void 
world_manager::PlayerAfterSwitch(gplayer_imp * pImp)
{
	pImp->ClearSwitchAdditionalData();
}

bool 
world_manager::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & world_tag)
{
	return player_template::GetTownPosition(opos,pos,world_tag);
}

void 
world_manager::RecordMob(int type, int tid, const A3DVECTOR &pos,int faction,int cnt)
{
	if(type == 1)
	{
		//只记录NPC
		w_service_npc_list[tid] = pos;
	}
	else
	{
		w_normal_mobs_list[tid] = pos;
	}
}


static std::set<int>  __rare_list;

bool world_manager::IsRareItem(int item_id)
{
	return __rare_list.find(item_id) != __rare_list.end();
}

bool world_manager::LoadLuaVersion(const char * filename)
{
	char buf[1024];
	FILE * file = fopen(filename, "rb");
	if(file == NULL) return false;
	bool bRst = false;
	if(fgets(buf,sizeof(buf), file)) 
	{
		int version = 0;
		sscanf(buf, "--%d", &version);
		if(version <= 0)
		{
			__PRINTINFO("LUA腳本錯誤%d '%s'\n", version, buf);
		}
		else
		{
			_lua_data_version = version;
			__PRINTINFO("LUA腳本版本%d\n", version);
			bRst = true;
		}
	}
	fclose(file);
	return bRst;
}

bool world_manager::LoadMallConsumptionConfig()
{
	itemdataman & dataman = GetDataMan();
	DATA_TYPE dt;
	CONSUME_POINTS_CONFIG * config = (CONSUME_POINTS_CONFIG*) dataman.get_data_ptr(1024, ID_SPACE_CONFIG, dt);
	if(config == NULL || dt != DT_CONSUME_POINTS_CONFIG)
	{
		return false;
	}

	std::set<int> allids;

	for(size_t i = 0; i < sizeof(config->list) / sizeof(config->list[0]); i++)
	{
		int id = config->list[i].id;
		if (id <= 0)
		{
			break;
		}

		_consumption_items_shopping.insert(id);

		if (allids.find(id) != allids.end())
		{
			printf("消费值配置表：重复ID：%d\n", id);
			return false;
		}
		else
		{
			allids.insert(id);
		}
	}

	config = (CONSUME_POINTS_CONFIG*) dataman.get_data_ptr(1026, ID_SPACE_CONFIG, dt);
	if(config == NULL || dt != DT_CONSUME_POINTS_CONFIG)
	{
		return false;
	}

	for(size_t i = 0; i < sizeof(config->list) / sizeof(config->list[0]); i++)
	{
		int id = config->list[i].id;
		if (id <= 0)
		{
			break;
		}

		int point = config->list[i].point;
		if (point <= 0)
		{
			printf("消费值配置表：错误的值 id=%d, value=%d\n", id, point);
			return false;
		}

		_consumption_items_binding[id] = point;

		if (allids.find(id) != allids.end())
		{
			printf("消费值配置表：重复ID：%d\n", id);
			return false;
		}
		else
		{
			allids.insert(id);
		}
	}

	config = (CONSUME_POINTS_CONFIG*) dataman.get_data_ptr(1025, ID_SPACE_CONFIG, dt);
	if(config == NULL || dt != DT_CONSUME_POINTS_CONFIG)
	{
		return false;
	}

	for(size_t i = 0; i < sizeof(config->list) / sizeof(config->list[0]); i++)
	{
		int id = config->list[i].id;
		if (id <= 0)
		{
			break;
		}

		int point = config->list[i].point;
		if (point <= 0)
		{
			printf("消费值配置表：错误值 id=%d, value=%d\n", id, point);
			return false;
		}

		_consumption_items_destroying[id] = point;

		if (allids.find(id) != allids.end())
		{
			printf("消费值配置表：重复ID：%d\n", id);
			return false;
		}
		else
		{
			allids.insert(id);
		}
	}

	return true;
}

void world_manager::LoadRareItemList(const char * filename)
{
	char buf[256];
	FILE * file = fopen(filename, "rb");
	if(file == NULL) return;
	while(fgets(buf,sizeof(buf),file))
	{
		abase::strtok tok(buf," \t\r\n,|\"'[]/");
		const char * token;
		while((token = tok.token()))
		{       
			if(!*token) continue;
			int id = atoi(token);
			if(id > 0)
			{
				if(!__rare_list.insert(id).second)
				{
					__PRINTINFO("稀有物品%d被重复加入\n",id);
				}
			}
			
		}
	}
	fclose(file);
	__PRINTINFO("总共加入了%d个稀有物品\n",__rare_list.size());

	{
		itemdataman & dataman = GetDataMan();
		int cash_item_count = 0;
		DATA_TYPE  dt;
		size_t id = dataman.get_first_data_id(ID_SPACE_ESSENCE,dt);
		for(; id != 0; id = dataman.get_next_data_id(ID_SPACE_ESSENCE,dt))
		{ 
			const item_data * pItem = (const item_data *)dataman.get_item_for_sell(id);
			if(!pItem) continue;
			if(pItem->proc_type & item::ITEM_PROC_TYPE_CASHITEM)
			{
				SetCashItem(id);
				cash_item_count ++;
				//printf("人民币物品%d\n",id);
			}
		}
		__PRINTINFO("总共存在%d个人民币物品\n", cash_item_count);
	}

	{
		DATA_TYPE dt;
		PLAYER_DEATH_DROP_CONFIG * cfg = (PLAYER_DEATH_DROP_CONFIG *)world_manager::GetDataMan().get_data_ptr(1022,ID_SPACE_CONFIG,dt);
		if(cfg && dt == DT_PLAYER_DEATH_DROP_CONFIG)
		{
			for(size_t i=0; i<sizeof(cfg->itemlist)/sizeof(cfg->itemlist[0]); i++)
			{
				if(!cfg->itemlist[i]) break;
				SetDeathDropItem(cfg->itemlist[i]);
			}	
		}
	}
}

enum {CASH_MAx_ITEM_COUNT = 65535};
static char __cash_item_list[CASH_MAx_ITEM_COUNT];
void world_manager::SetCashItem(size_t id)
{
	if(id >= CASH_MAx_ITEM_COUNT)
	{
		ASSERT(false);
		return;
	}
	__cash_item_list[id] = 1;
}

void world_manager::TestCashItemGenerated(size_t id,int count)
{
	if(id >= CASH_MAx_ITEM_COUNT) return;
	if(!__cash_item_list[id]) return ;
	GLog::cash("cash_item_generated:item_id=%d:count=%d:", id, count);
}

static hash_map<int, int> __death_drop_item_list;

void world_manager::SetDeathDropItem(int item_id)
{
	__death_drop_item_list[item_id] = 1;
}

bool world_manager::IsDeathDropItem(int item_id)
{
	return (__death_drop_item_list.find(item_id) != __death_drop_item_list.end());
}

void world_manager::LoadServerData()
{
	class ServerDataLoadResult : public abase::ASmallObject, public GDB::ServerDataResult
	{
		world_manager * pMan;
	public:
		ServerDataLoadResult(world_manager * man) : pMan(man){}
		virtual void OnTimeOut()
		{
			//rpc超时，由load_task负责重新发送rpc
			GLog::log(GLOG_ERR,"从数据库取得server data超时,worldtag=%d", world_manager::GetWorldTag());
			delete this;
		}
		virtual void OnFailed()
		{
			//失败了，释放掉load_task，此时使用serverdata的系统均不可用
			GLog::log(GLOG_ERR,"从数据库取得server data失败,worldtag=%d", world_manager::GetWorldTag());
			if(pMan->_load_task)
			{
				pMan->_load_task->RemoveTimer();
				delete pMan->_load_task;
				pMan->_load_task = NULL;
			}
			delete this;
		}
		virtual void OnGetServerData(int world_tag, const GDB::serverdata * data, int mask)
		{
			//成功了，释放掉load_task，开始初始化各个使用serverdata的系统
			//如果某个系统初始化失败，忽略此错误并且该系统不可用
			//至少有一个系统初始化成功才设置_serverdata_init，设置此标志则执行serverdata的自动存盘
			GLog::log(GLOG_INFO,"从数据库取得server data成功,worldtag=%d", world_manager::GetWorldTag());
		
			if(mask & DBMASK_SERVERDATA_WEDDING)
			{
				raw_wrapper ar(data->wedding_data.data,data->wedding_data.size);
				if(!pMan->WeddingDBLoad(ar))
				{
					GLog::log(GLOG_ERR, "婚礼系统加载数据失败. world_tag=%d",world_manager::GetWorldTag());
					mask &= ~DBMASK_SERVERDATA_WEDDING;
				}
			}
			if(mask & DBMASK_SERVERDATA_DPSRANK)
			{
				raw_wrapper ar(data->dpsrank_data.data, data->dpsrank_data.size);
				if(!pMan->DpsRankDBLoad(ar))
				{
					GLog::log(GLOG_ERR, "dps排行榜系统加载数据失败. world_tag=%d",world_manager::GetWorldTag());
					mask &= ~DBMASK_SERVERDATA_DPSRANK;
				}

			}

			if(mask)
				pMan->_serverdata_init = true;
			if(pMan->_load_task)
			{
				pMan->_load_task->RemoveTimer();
				delete pMan->_load_task;
				pMan->_load_task = NULL;
			}
			delete this;
		}
	};

	int mask = 0;
	if(IsWeddingServer()) mask |= DBMASK_SERVERDATA_WEDDING;
	if(HasDpsRank()) mask |= DBMASK_SERVERDATA_DPSRANK;
	ASSERT(mask);

	GDB::get_serverdata(_world_tag, new ServerDataLoadResult(this), mask);
}

void world_manager::AutoSaveServerData()
{
	class ServerDataAutoSaveResult : public abase::ASmallObject, public GDB::ServerDataResult
	{
		world_manager * pMan;
	public:
		ServerDataAutoSaveResult(world_manager * man) : pMan(man){}
		virtual void OnTimeOut()
		{
			//暂时先不处理
			//貌似停掉相应功能比较好
			GLog::log(GLOG_ERR,"向数据库保存server data超时,worldtag=%d", world_manager::GetWorldTag());
			delete this;
		}
		virtual void OnFailed()
		{
			//暂时先不处理
			//貌似停掉相应功能比较好
			GLog::log(GLOG_ERR,"向数据库保存server data失败,worldtag=%d", world_manager::GetWorldTag());
			delete this;
		}
		virtual void OnPutServerData()
		{
			GLog::log(GLOG_INFO,"向数据库保存server data成功,worldtag=%d", world_manager::GetWorldTag());
			delete this;
		}
	};

	int mask = 0;
	GDB::serverdata data;
	memset(&data,0,sizeof(data));
	data.world_tag = _world_tag;
	
	raw_wrapper wedding_ar;
	if(IsWeddingServer()) 
	{	
		if(WeddingDBSave(wedding_ar))
		{
			mask |= DBMASK_SERVERDATA_WEDDING;
			data.wedding_data.data = wedding_ar.data();
			data.wedding_data.size = wedding_ar.size();
		}
	}
	raw_wrapper dpsrank_ar;
	if(HasDpsRank()) 
	{	
		if(DpsRankDBSave(dpsrank_ar))
		{
			mask |= DBMASK_SERVERDATA_DPSRANK;
			data.dpsrank_data.data = dpsrank_ar.data();
			data.dpsrank_data.size = dpsrank_ar.size();
		}
	}
	ASSERT(mask);
	
	GDB::put_serverdata(_world_tag, &data, new ServerDataAutoSaveResult(this), 1, mask);
}

void world_manager::ShutDownSaveServerData()
{
	int mask = 0;
	GDB::serverdata data;
	memset(&data,0,sizeof(data));
	data.world_tag = _world_tag;
	
	raw_wrapper wedding_ar;
	if(IsWeddingServer()) 
	{	
		if(WeddingDBSave(wedding_ar))
		{
			mask |= DBMASK_SERVERDATA_WEDDING;
			data.wedding_data.data = wedding_ar.data();
			data.wedding_data.size = wedding_ar.size();
		}
	}
	raw_wrapper dpsrank_ar;
	if(HasDpsRank()) 
	{	
		if(DpsRankDBSave(dpsrank_ar))
		{
			mask |= DBMASK_SERVERDATA_DPSRANK;
			data.dpsrank_data.data = dpsrank_ar.data();
			data.dpsrank_data.size = dpsrank_ar.size();
		}
	}
	ASSERT(mask);
	
	GDB::put_serverdata(_world_tag, &data, NULL, 2, mask);
}

int world_manager::ReceiveMessage(int msg_tag,const MSG & message)
{
	class extern_message_task : public ONET::Thread::Runnable
	{
		world_message_handler * _pHandler;
		int _msg_tag;
		MSG * _pMsg;
		public:
		extern_message_task(world_message_handler * handler, int msg_tag, const MSG & msg):_pHandler(handler),_msg_tag(msg_tag)
		{
			_pMsg = DupeMessage(msg);
		}
		~extern_message_task()
		{
			if(_pMsg) FreeMessage(_pMsg);	
		}
		virtual void Run()
		{       
			_pHandler->RecvExternMessage(_msg_tag,*_pMsg);
			delete this;
		}       
	};
	switch(message.message)
	{
		//耗时比较多的msg移入线程池处理。
		case GM_MSG_SWITCH_USER_DATA:
			ONET::Thread::Pool::AddTask(new extern_message_task(_message_handler,msg_tag,message));
			return 0;

		default:
			return _message_handler->RecvExternMessage(msg_tag,message);
	}
}

bool world_manager::InitRegionWayPointMap()
{
    abase::vector<TRANS_TARGET_SERV>& targets = globaldata_gettranstargetsserver();
    if (targets.empty()) return true;

    for (unsigned int i = 0; i < targets.size(); ++i)
    {
        TRANS_TARGET_SERV& tar = targets[i];
        if (tar.world_tag != _world_tag) continue;

        CELRegion* pRegion = city_region::GetRegion(tar.vecPos.x, tar.vecPos.z);
        if (pRegion == NULL) 
        {
            printf("=============warning, invalid waypoint in world=%d, waypoint_id=%d, waypoint_pos=(%f %f %f)\n", 
                _world_tag, tar.id, tar.vecPos.x, tar.vecPos.y, tar.vecPos.z);
            continue;
        }
        
        int region_id = pRegion->GetID();
        _region_waypoint_map[region_id].push_back(tar.id);
    }
    
    return true;
}

