#include "../world.h"
#include "faction_world_ctrl.h"
#include <factionlib.h> 

void faction_world_ctrl::Init(world * pPlane, const GNET::faction_fortress_data * data,const GNET::faction_fortress_data2 * data2)
{
	time_t t = time(NULL);
	bool new_week = false;
	factionid 	= data->factionid;	
	level 		= data->level;	
	exp 		= data->exp;	
	exp_today 	= data->exp_today;	
	exp_today_time = data->exp_today_time;	
	if(t - exp_today_time >= 86400)
	{
		struct tm & tm1 = *localtime(&t);
		int week_begin = t - 86400*tm1.tm_wday - 3600*tm1.tm_hour - 60*tm1.tm_min - tm1.tm_sec;
		if(tm1.tm_wday <= 2) week_begin -= 4*86400;	//周三零点为周起始
		else week_begin += 3*86400;
		new_week = (week_begin > exp_today_time);
		exp_today_time = t - tm1.tm_hour*3600 - tm1.tm_min*60 - tm1.tm_sec;
		exp_today = 0;
	}
	tech_point 	= data->tech_point;	
	if(data->technology.size == sizeof(int)*5)
	{
		memcpy(technology,data->technology.data,data->technology.size);	
	}
	else
	{
		ASSERT(false);
	}
	if(data->material.size == sizeof(int)*8)
	{
		memcpy(material,data->material.data,data->material.size);	
	}
	else
	{
		ASSERT(false);
	}
	if(data->building.size % (sizeof(int)*2) == 0)
	{
		building_count = 0;
		int * tmp = (int *)data->building.data;
		size_t size = data->building.size/sizeof(int);
		for(size_t i=0; i<size; i+=2)
		{
			int id = tmp[i];
			int finish_time = tmp[i+1];
			if(finish_time != 0 && t >= finish_time) finish_time = 0;
			
			DATA_TYPE dt;
			FACTION_BUILDING_ESSENCE * ess = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(id,ID_SPACE_ESSENCE,dt);
			if(!ess || dt != DT_FACTION_BUILDING_ESSENCE) continue;

			building[building_count].id = id;
			building[building_count].finish_time = finish_time;
			++ building_count;

			if(finish_time != 0)
				pPlane->TriggerSpawn(ess->controller_id0, false);	
			else
				pPlane->TriggerSpawn(ess->controller_id1, false);	
		}
	}
	else
	{
		ASSERT(false);
	}
	if(new_week)
	{
		//新的一周了，重置全局变量和控制器为预设值	
		ResetCommonValueAndSpawner(pPlane);	
	}
	else
	{
		if(data->common_value.size % (sizeof(int)*2) == 0)
		{
			int * tmp = (int *)data->common_value.data;
			size_t size = data->common_value.size/sizeof(int);
			for(size_t i=0; i<size; i+=2)
			{
				int key = tmp[i];
				int value = tmp[i+1];
				if(key >= SAVED_COMMON_VALUE_START && key <= SAVED_COMMON_VALUE_END)
				{
					pPlane->SetCommonValue(key,value,false);		
					common_value[key-SAVED_COMMON_VALUE_START] = value;
				}
			}
		}
		else
		{
			ASSERT(false);
		}
		if(data->actived_spawner.size % sizeof(int) == 0)
		{
			int * tmp = (int *)data->actived_spawner.data;
			size_t size = data->actived_spawner.size/sizeof(int);
			for(size_t i=0; i<size; i++)
			{
				int controller_id = tmp[i];
				if(controller_id >= SAVED_ACTIVED_SPAWNER_START && controller_id <= SAVED_ACTIVED_SPAWNER_END)
				{
					pPlane->TriggerSpawn(controller_id, false);
					actived_spawner[controller_id-SAVED_ACTIVED_SPAWNER_START] = 1;
				}
			}
		}
		else
		{
			ASSERT(false);
		}
	}
	
	health 		= data2->health;	
	offense_faction = data2->offense_faction;
	offense_starttime = data2->offense_starttime;
	offense_endtime = data2->offense_endtime;
	ASSERT(health > 0);

	write_timer = abase::Rand(45,55);		
	inbattle = (t >= offense_starttime && t < offense_endtime);
	if(inbattle)
	{
		player_count_limit = PLAYER_LIMIT_IN_BATTLE;
	}
	else
	{
		player_count_limit = 999;
	}
	if(t >= offense_starttime - 300 && t < offense_starttime)
	{
		int tmp = offense_starttime - t;
		if(tmp%10 == 0)
			iskick = tmp/10;
		else
			iskick = tmp/10 + 1;
	}
	//通知deliveryd帮派基地已开启
	GNET::SendFactionFortressState(factionid, 1);
	__PRINTF("++++++++++++++faction_world_ctrl::init factionid=%d iskick=%d\n",factionid,iskick);
}

void faction_world_ctrl::OnNotifyData(world * pPlane, const GNET::faction_fortress_data2 * data2)
{
	spin_autolock keeper(lock);
	health 		= data2->health;	
	offense_faction = data2->offense_faction;
	offense_starttime = data2->offense_starttime;
	offense_endtime = data2->offense_endtime;
	if(health <= 0)
		iskick = 360;	//60分钟
}

bool faction_world_ctrl::LevelUp()
{
	spin_autolock keeper(lock);
	if(level >= MAX_FORTRESS_LEVEL) return false;
	DATA_TYPE dt;
	FACTION_FORTRESS_CONFIG * cfg = (FACTION_FORTRESS_CONFIG *)world_manager::GetDataMan().get_data_ptr(FACTION_FORTRESS_CONFIG_ID,ID_SPACE_CONFIG,dt);
	if(!cfg || dt != DT_FACTION_FORTRESS_CONFIG) return false;
	int levelup_exp = cfg->level[level-1].exp; 
	if(levelup_exp <= 0) return false;
	if(exp < levelup_exp) return false;
	exp -= levelup_exp;
	++ level;
	tech_point += cfg->level[level-1].tech_point;

	GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_LEVELUP,&level,sizeof(int));
	return true;
}

bool faction_world_ctrl::SetTechPoint(size_t tech_index)
{
	spin_autolock keeper(lock);
	if(tech_index >= TECHNOLOGY_COUNT) return false;
	if(technology[tech_index] >= MAX_TECH_LEVEL) return false;
	DATA_TYPE dt;
	FACTION_FORTRESS_CONFIG * cfg = (FACTION_FORTRESS_CONFIG *)world_manager::GetDataMan().get_data_ptr(FACTION_FORTRESS_CONFIG_ID,ID_SPACE_CONFIG,dt);
	if(!cfg || dt != DT_FACTION_FORTRESS_CONFIG) return false;
	int cost = cfg->tech_point_cost[tech_index][technology[tech_index]];
	if(cost <= 0) return false;
	if(tech_point < cost) return false;
	tech_point -= cost;
	++ technology[tech_index];
	
	struct
	{
		size_t tech_index;
		int tech_level;
	}data;
	data.tech_index = tech_index;
	data.tech_level = technology[tech_index];
	GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_TECHNOLOGYUP,&data,sizeof(data));
	return true;
}

bool faction_world_ctrl::ResetTechPoint(world * pPlane, size_t tech_index)
{
	spin_autolock keeper(lock);
	if(tech_index >= TECHNOLOGY_COUNT) return false;
	if (technology[tech_index] <= 0) return false;
	
	DATA_TYPE dt;
	FACTION_FORTRESS_CONFIG * cfg = (FACTION_FORTRESS_CONFIG *)world_manager::GetDataMan().get_data_ptr(FACTION_FORTRESS_CONFIG_ID,ID_SPACE_CONFIG,dt);
	if(!cfg || dt != DT_FACTION_FORTRESS_CONFIG) return false;

	//计算该系升级到现在的科技等级所需要的总点数，需要返回给他
	int total_tech = 0;
	for (int i = 0; i < technology[tech_index]; i++)
	{
		total_tech += cfg->tech_point_cost[tech_index][i];
	}
	if (total_tech <= 0) return false;

	//所有检查通过了，开始重置科技等级，并返回科技点数
	technology[tech_index] = 0;
	tech_point += total_tech;

	//需要把所有用到该系科技的建筑全部删掉
	for (int i=0; i < building_count; i++)
	{
		DATA_TYPE dt2;
		int id = building[i].id;
		FACTION_BUILDING_ESSENCE * ess = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(id,ID_SPACE_ESSENCE,dt2);	
		ASSERT(ess && dt2 == DT_FACTION_BUILDING_ESSENCE);
		if (ess->technology[tech_index] > 0)
		{
			//不论是否正在建筑，全部删掉控制器
			if (building[i].finish_time != 0)
				pPlane->ClearSpawn(ess->controller_id0, false);	
			else
				pPlane->ClearSpawn(ess->controller_id1, false);
			//删掉building的信息
			building[i].id = building[building_count-1].id;
			building[i].finish_time = building[building_count-1].finish_time;
			building[building_count-1].id = 0;
			building[building_count-1].finish_time = 0;
			-- building_count;
			//广播给帮派玩家
			struct
			{
				int id;
				int type;	//1 是玩家主动拆除，回收材料 2是系统拆除，不回收材料
			}data;
			data.id = id;
			data.type = 2;
			GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_DISMANTLE,&data,sizeof(data));
		}
	}
	return true;
}

bool faction_world_ctrl::Construct(world * pPlane, int id, int accelerate)
{
	spin_autolock keeper(lock);
	if(inbattle) return false;
	if(accelerate < 0 || accelerate > 20) return false;
	DATA_TYPE dt;
	FACTION_BUILDING_ESSENCE * ess = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(id,ID_SPACE_ESSENCE,dt);
	if(!ess || dt != DT_FACTION_BUILDING_ESSENCE) return false;
	if(level < ess->require_level) return false;
	for(int i=0; i<TECHNOLOGY_COUNT; i++)
	{
		if(technology[i] < ess->technology[i]) return false;
	}
	for(int i=0; i<MATERIAL_COUNT; i++)
	{
		if(material[i] < ess->material[i]*(10+accelerate)/10) return false;
	}
	int finish_time = 0;
	if(ess->base_time - ess->delta_time*accelerate > 0)
		finish_time = time(NULL) + ess->base_time - ess->delta_time*accelerate;
	int overlap_index = -1;
	FACTION_BUILDING_ESSENCE * overlap_ess = NULL;
	for(int i=0; i<building_count; i++)
	{
		DATA_TYPE dt2;
		FACTION_BUILDING_ESSENCE * ess2 = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(building[i].id,ID_SPACE_ESSENCE,dt2);	
		ASSERT(ess2 && dt2 == DT_FACTION_BUILDING_ESSENCE);
		if(ess2->id_sub_type != ess->id_sub_type) continue;
		if(ess2->level >= ess->level) return false;
		if(overlap_index != -1) return false;
		overlap_index = i;
		overlap_ess = ess2;
	}
	if(overlap_index == -1)
	{
		if(building_count == BUILDING_MAX) return false;
		building[building_count].id = id;
		building[building_count].finish_time = finish_time;
		++ building_count;
		if(finish_time != 0)
			pPlane->TriggerSpawn(ess->controller_id0, false);	
		else
			pPlane->TriggerSpawn(ess->controller_id1, false);	
	}
	else
	{
		if(building[overlap_index].finish_time != 0)
			pPlane->ClearSpawn(overlap_ess->controller_id0, false);
		else
			pPlane->ClearSpawn(overlap_ess->controller_id1, false);
		building[overlap_index].id = id;
		building[overlap_index].finish_time = finish_time;
		if(finish_time != 0)
			pPlane->TriggerSpawn(ess->controller_id0, false);	
		else
			pPlane->TriggerSpawn(ess->controller_id1, false);	
	}
	for(int i=0; i<MATERIAL_COUNT; i++)
	{
		material[i] -= ess->material[i]*(10+accelerate)/10;
		if(overlap_ess)	material[i] += overlap_ess->material[i]/2;
	}
	
	struct
	{
		int id;
		int accelerate;
		int overlap_id;
	}data;
	data.id = id;
	data.accelerate = accelerate;
	data.overlap_id = (overlap_ess ? overlap_ess->id : 0);
	GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_CONSTRUCT,&data,sizeof(data));
	return true;
}

static int GetMaterialLimit(int level)
{
	static const int table[] = {10000,20000,40000,60000,80000,100000,120000};
	int index = level/5;
	if(index < 0) index = 0;
	if((size_t)index > sizeof(table)/sizeof(int)-1) index = sizeof(table)/sizeof(int)-1;
	return table[index];
}

bool faction_world_ctrl::HandInMaterial(int id, size_t count)
{
	spin_autolock keeper(lock);
	DATA_TYPE dt;
	FACTION_MATERIAL_ESSENCE * ess = (FACTION_MATERIAL_ESSENCE *)world_manager::GetDataMan().get_data_ptr(id,ID_SPACE_ESSENCE,dt);
	if(!ess || dt != DT_FACTION_MATERIAL_ESSENCE) return false;
	int tmp[MATERIAL_COUNT] = {0};
	for(int i=0; i<MATERIAL_COUNT; i++)
	{
		tmp[i] = material[i] + count * ess->material_count[i];
		if(tmp[i] == material[i]) continue;
		if(tmp[i] < material[i] || tmp[i] > GetMaterialLimit(level)) return false;
	}
	for(int i=0; i<MATERIAL_COUNT; i++)
	{
		material[i] = tmp[i];
	}
	return true;
}

bool faction_world_ctrl::HandInContrib(int contrib)
{
	spin_autolock keeper(lock);
	if(level >= MAX_FORTRESS_LEVEL) return false;
	int inc_exp = (int)(contrib / 100.f * (health>=80 ? health : 80));
	int tmp = exp + inc_exp;
	if(tmp <= exp) return false;
	int tmp2 = exp_today + inc_exp;
	if(tmp2 <= exp_today) return false;
	exp = tmp;
	exp_today = tmp2;
	return true;
}

bool faction_world_ctrl::MaterialExchange(size_t src_index,size_t dst_index,int count)
{
	spin_autolock keeper(lock);
	if(src_index >= MATERIAL_COUNT || dst_index >= MATERIAL_COUNT || src_index == dst_index || count <= 0) return false;
	if(material[src_index] < count) return false;
	int new_count = count/2;
	int tmp = material[dst_index] + new_count;
	if(tmp <= material[dst_index] || tmp > GetMaterialLimit(level)) return false;
	material[dst_index] = tmp;
	material[src_index] -= count;
	return true;
}

bool faction_world_ctrl::Dismantle(world * pPlane, int id)
{
	spin_autolock keeper(lock);
	if(inbattle) return false;
	int i = 0;
	for( ; i<building_count; i++)
	{
		if(building[i].id == id)
		{
			break;
		}
	}
	if(i != building_count)
	{
		DATA_TYPE dt;
		FACTION_BUILDING_ESSENCE * ess = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(building[i].id,ID_SPACE_ESSENCE,dt);
		ASSERT(ess && dt == DT_FACTION_BUILDING_ESSENCE);
		if(building[i].finish_time) return false;	
		pPlane->ClearSpawn(ess->controller_id1, false);	
		building[i].id = building[building_count-1].id;
		building[i].finish_time = building[building_count-1].finish_time; 
		building[building_count-1].id = 0;
		building[building_count-1].finish_time = 0;
		-- building_count;

		for(int j=0; j<MATERIAL_COUNT; j++)
		{
			int tmp = material[j] + ess->material[j]/2;
			if(tmp <= material[j]) continue;
			if(tmp > GetMaterialLimit(level)) tmp = GetMaterialLimit(level);
			material[j] = tmp;
		}
		struct
		{
			int id;
			int type;	//1 是玩家主动拆除，回收材料 2是系统拆除，不回收材料
		}data;
		data.id = id;
		data.type = 1;
		GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_DISMANTLE,&data,sizeof(data));
		return true;
	}
	return false;
}

bool faction_world_ctrl::GetInfo(int roleid, int cs_index, int cs_sid)
{
	spin_autolock keeper(lock);
	packet_wrapper h1(256);
	using namespace S2C;
	CMD::Make<CMD::faction_fortress_info>::From(h1);
	h1 << factionid << level << exp << exp_today << exp_today_time << tech_point;
	for(int i=0; i<TECHNOLOGY_COUNT; i++)
		h1 << technology[i];
	for(int i=0; i<MATERIAL_COUNT; i++)
		h1 << material[i];
	h1 << building_count;
	for(int i=0; i<building_count; i++)
		h1 << building[i].id << building[i].finish_time;
	h1 << health;
	send_ls_msg(cs_index,roleid,cs_sid,h1);
	return true;
}

void faction_world_ctrl::OnBuildingDestroyed(world * pPlane, int id)
{
	spin_autolock keeper(lock);
	int i = 0;
	for( ; i<building_count; i++)
	{
		if(building[i].id == id)
		{
			break;
		}
	}
	if(i != building_count)
	{
		//找到了
		DATA_TYPE dt;
		FACTION_BUILDING_ESSENCE * ess = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(building[i].id,ID_SPACE_ESSENCE,dt);
		ASSERT(ess && dt == DT_FACTION_BUILDING_ESSENCE);
		if(building[i].finish_time)
			pPlane->ClearSpawn(ess->controller_id0, false);	
		else
			pPlane->ClearSpawn(ess->controller_id1, false);	
		building[i].id = building[building_count-1].id;
		building[i].finish_time = building[building_count-1].finish_time; 
		building[building_count-1].id = 0;
		building[building_count-1].finish_time = 0;
		-- building_count;

		if(ess->id_sub_type == 29550)	//主基地建筑
		{
			struct
			{
				int factionid;
				int offense_faction;
			}data;
			data.factionid = factionid;
			data.offense_faction = offense_faction;
			broadcast_chat_msg(GMSV::CMSG_FF_KEYBUILDINGDESTROY,&data,sizeof(data),GMSV::CHAT_CHANNEL_SYSTEM,0,NULL,0);
		}
	}
}

void faction_world_ctrl::Reset()
{
	spin_autolock keeper(lock);
	//帮派退出前先进行存盘操作
	SaveFactionData(NULL);
	//通知deliveryd帮派基地已关闭
	GNET::SendFactionFortressState(factionid, 0);

	tick_counter = write_timer = lock = 0;
	iskick = factionid = level = exp = exp_today = exp_today_time = tech_point = building_count = 0;
	health = offense_faction = offense_starttime = offense_endtime = 0;
	inbattle = false;	
	player_count_limit = 999;
	defender_count = attacker_count = _user_list_lock = 0;
	memset(technology,0,sizeof(technology));
	memset(material,0,sizeof(material));
	memset(building,0,sizeof(building));
	memset(common_value,0,sizeof(common_value));
	memset(actived_spawner,0,sizeof(actived_spawner));
	_attacker_list.clear();
	_defender_list.clear();
	_all_list.clear();
}

void faction_world_ctrl::Tick(world * pPlane)
{
	spin_autolock keeper(lock);
	if(++ tick_counter < 200) return;	//10秒一次
	tick_counter = 0;
	
	time_t t = time(NULL);
	bool new_week = false;
	//检查当天经验
	if(t - exp_today_time >= 86400)
	{
		struct tm & tm1 = *localtime(&t);
		int week_begin = t - 86400*tm1.tm_wday - 3600*tm1.tm_hour - 60*tm1.tm_min - tm1.tm_sec;
		if(tm1.tm_wday <= 2) week_begin -= 4*86400;
		else week_begin += 3*86400;
		new_week = (week_begin > exp_today_time);
		exp_today_time += 86400;
		exp_today = 0;
	}
	//
	if(new_week)
	{
		//新的一周了，重置全局变量和控制器为预设值	
		ResetCommonValueAndSpawner(pPlane);	
	}
	//检查建筑完成
	for(int i=0; i<building_count; i++)
	{
		if(building[i].finish_time != 0 && t > building[i].finish_time)
		{
			building[i].finish_time = 0;	
			DATA_TYPE dt;
			FACTION_BUILDING_ESSENCE * ess = (FACTION_BUILDING_ESSENCE *)world_manager::GetDataMan().get_data_ptr(building[i].id,ID_SPACE_ESSENCE,dt);
			if(!ess || dt != DT_FACTION_BUILDING_ESSENCE) continue;
			pPlane->ClearSpawn(ess->controller_id0, false);	
			pPlane->TriggerSpawn(ess->controller_id1, false);	
			
			GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_CONSTRUCTCOMPLETE,&(building[i].id),sizeof(int));
		}
	}
	//检查存盘
	if(--write_timer <= 0)
	{
		class PutFactionFortress : public GNET::FactionFortressResult
		{
			int _factionid;
		public:
			PutFactionFortress(int factionid):_factionid(factionid)
			{}

			virtual void OnTimeOut()
			{
				GLog::log(GLOG_ERR,"向数据库保存faction data失败,factionid=%d", _factionid);
				delete this;
			}
			virtual void OnFailed()
			{
				GLog::log(GLOG_ERR,"向数据库保存faction data失败,factionid=%d", _factionid);
				delete this;
			}
			virtual void OnPutData()
			{
				GLog::log(GLOG_INFO,"向数据库保存faction data成功,factionid=%d", _factionid);
				delete this;
			}
		};
	
		SaveFactionData(new PutFactionFortress(factionid));
		write_timer = abase::Rand(45,55);		
	}
	//
	inbattle = (t >= offense_starttime && t < offense_endtime);
	if(inbattle)
	{
		player_count_limit = PLAYER_LIMIT_IN_BATTLE;	
	}
	else
	{
		player_count_limit = 999;
	}
	
	if(iskick > 0)
		-- iskick;
	else if(t >= offense_starttime - 300 && t < offense_starttime)	
	{
		iskick = 30;	//5分钟
		GMSV::FactionBroadcastMsg(factionid,GMSV::CMSG_FF_BATTLEPREPARECLEAR,NULL,0);
		__PRINTF("++++++++++++++faction_world_ctrl::tick factionid=%d iskick=%d\n",factionid,iskick);
	}
}

void faction_world_ctrl::OnSetCommonValue(int key, int value)
{
	spin_autolock keeper(lock);
	if(key >= SAVED_COMMON_VALUE_START && key <= SAVED_COMMON_VALUE_END)
		common_value[key-SAVED_COMMON_VALUE_START] = value;
}

void faction_world_ctrl::OnTriggerSpawn(int controller_id)
{
	spin_autolock keeper(lock);
	if(controller_id >= SAVED_ACTIVED_SPAWNER_START && controller_id <= SAVED_ACTIVED_SPAWNER_END)
		actived_spawner[controller_id-SAVED_ACTIVED_SPAWNER_START] = 1;
}

void faction_world_ctrl::OnClearSpawn(int controller_id)
{
	spin_autolock keeper(lock);
	if(controller_id >= SAVED_ACTIVED_SPAWNER_START && controller_id <= SAVED_ACTIVED_SPAWNER_END)
		actived_spawner[controller_id-SAVED_ACTIVED_SPAWNER_START] = 0;
}

void faction_world_ctrl::OnServerShutDown()
{
	spin_autolock keeper(lock);
	SaveFactionData(NULL);
	//通知deliveryd帮派基地已关闭
	GNET::SendFactionFortressState(factionid, 0);
}

void faction_world_ctrl::SaveFactionData(GNET::FactionFortressResult * callback)
{
	ASSERT(lock);
	GNET::faction_fortress_data data;
	data.factionid 	= factionid;
	data.level 		= level;
	data.exp 		= exp;
	data.exp_today 	= exp_today;
	data.exp_today_time = exp_today_time;
	data.tech_point = tech_point;
	data.technology.data = technology;
	data.technology.size = sizeof(int)*TECHNOLOGY_COUNT;
	data.material.data = material;
	data.material.size = sizeof(int)*MATERIAL_COUNT;
	data.building.data = building;
	data.building.size = sizeof(int)*2*building_count;
	size_t cv_cnt = 0;
	int cv[(SAVED_COMMON_VALUE_END-SAVED_COMMON_VALUE_START+1)*2];
	for(size_t i=0; i<SAVED_COMMON_VALUE_END-SAVED_COMMON_VALUE_START+1; i++)
	{
		if(common_value[i] != 0)
		{
			cv[2*cv_cnt] = SAVED_COMMON_VALUE_START+i;
			cv[2*cv_cnt+1] = common_value[i];
			++ cv_cnt;
		}
	}
	data.common_value.data = cv;
	data.common_value.size = sizeof(int)*2*cv_cnt;
	size_t as_cnt = 0;
	int as[SAVED_ACTIVED_SPAWNER_END-SAVED_ACTIVED_SPAWNER_START+1];
	for(size_t i=0; i<SAVED_ACTIVED_SPAWNER_END-SAVED_ACTIVED_SPAWNER_START+1; i++)
	{
		if(actived_spawner[i])
		{
			as[as_cnt] = SAVED_ACTIVED_SPAWNER_START+i;
			++ as_cnt;
		}
	}
	data.actived_spawner.data = as;
	data.actived_spawner.size = sizeof(int)*as_cnt;
	GNET::put_faction_fortress(factionid, &data, callback);
}

void faction_world_ctrl::ResetCommonValueAndSpawner(world * pPlane)
{
	//先重置
	for(size_t i=0; i<SAVED_COMMON_VALUE_END-SAVED_COMMON_VALUE_START+1; i++)
	{
		if(common_value[i] != 0)
		{
			pPlane->SetCommonValue(SAVED_COMMON_VALUE_START+i,0,false);
			common_value[i] = 0;
		}
	}
	for(size_t i=0; i<SAVED_ACTIVED_SPAWNER_END-SAVED_ACTIVED_SPAWNER_START+1; i++)
	{
		if(actived_spawner[i])
		{
			pPlane->ClearSpawn(SAVED_ACTIVED_SPAWNER_START+i, false);
			actived_spawner[i] = 0;
		}
	}
	//再从配置中读取
	DATA_TYPE dt;
	FACTION_FORTRESS_CONFIG * cfg = (FACTION_FORTRESS_CONFIG *)world_manager::GetDataMan().get_data_ptr(FACTION_FORTRESS_CONFIG_ID,ID_SPACE_CONFIG,dt);
	if(cfg && dt == DT_FACTION_FORTRESS_CONFIG)
	{
		for(size_t i=0; i<sizeof(cfg->controller_id)/sizeof(int); i++)
		{
			int controller_id = cfg->controller_id[i];
			if(controller_id <= 0) break;
			if(controller_id >= SAVED_ACTIVED_SPAWNER_START && controller_id <= SAVED_ACTIVED_SPAWNER_END)
			{
				pPlane->TriggerSpawn(controller_id, false);
				actived_spawner[controller_id-SAVED_ACTIVED_SPAWNER_START] = 1;
			}
		}
		for(size_t i=0; i<sizeof(cfg->common_value)/sizeof(cfg->common_value[0]); i++)
		{
			int key = cfg->common_value[i].id;
			int value = cfg->common_value[i].value;
			if(key <= 0) break;
			if(key >= SAVED_COMMON_VALUE_START && key <= SAVED_COMMON_VALUE_END)
			{
				pPlane->SetCommonValue(key,value,false);		
				common_value[key-SAVED_COMMON_VALUE_START] = value;
			}
		}
	}
	
}

void faction_world_ctrl::PlayerEnter(gplayer * pPlayer,int type)
{
	spin_autolock keeper(_user_list_lock);
	AddMapNode(_all_list,pPlayer);
	if(type & 0x01)
	{
		//attacker
		AddMapNode(_attacker_list,pPlayer);
	}
	else if(type & 0x02)
	{
		//defender
		AddMapNode(_defender_list,pPlayer);
	}
}

void faction_world_ctrl::PlayerLeave(gplayer * pPlayer,int type)
{
	spin_autolock keeper(_user_list_lock);
	DelMapNode(_all_list,pPlayer);
	if(type & 0x01)
	{
		//attacker
		DelMapNode(_attacker_list,pPlayer);
	}
	else if(type & 0x02)
	{
		//defender
		DelMapNode(_defender_list,pPlayer);
	}
}
