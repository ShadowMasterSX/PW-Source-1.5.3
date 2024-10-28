#include "string.h"
#include "world.h"
#include "strings.h"
#include <limits.h>
#include <conf.h>
#include "actobject.h"
#include "playertemplate.h"
#include "template/itemdataman.h"
#include "template/npcgendata.h"


int __allow_login_class_mask = 0xFFFFFFFF;
player_template player_template::_instance;
player_template::player_template():_generalcard_cls_adjust_list(USER_CLASS_COUNT, abase::vector<float>())
{
	memset(_template_list,0,sizeof(_template_list));
	memset(_class_list,0,sizeof(_class_list));
	memset(_level_adjust_table,0,sizeof(_level_adjust_table));
	memset(_team_adjust,0,sizeof(_team_adjust));
	memset(_team_race_adjust,0,sizeof(_team_race_adjust));
	memset(_use_soul_exp_adjust,0,sizeof(_use_soul_exp_adjust));
	dmg_adj_to_spec_atk_speed = 1.0f;
	atk_rate_adj_to_spec_atk_speed = 1.0f;
	for(int i = 0; i <= GetMaxLevelLimit(); i ++)
	{
		//放置基本的组队经验值
		//后面根据模板里的数据进行调整
		_exp_list.push_back(i*i*500);
		_pet_exp_list.push_back(i*i*500);
	}

	for(int i = 0; i < 256; i ++)
	{
		_death_exp_punish.push_back(0.05f);
	}
	_debug_mode = false;

	_exp_bonus = 0.f;
	_sp_bonus = 0.f;
	_money_bonus = 0.f;
}

bool
player_template::__Load(const char * filename,itemdataman * pDataMan)
{
	struct Data
	{
		static int ReadInt(ONET::Conf & conf, std::string section, std::string name)
		{
			std::string str;
			str = conf.find(section,name);
			if(str.empty()) {
				std::string err("Conf file error , in section '");
				err += section;
				err = err + "', with index name '";
				err += name;
				err = err + "'";
				throw err;
			}
			return atoi(str.c_str());
		}

		static double ReadFloat(ONET::Conf & conf, std::string section, std::string name)
		{
			std::string str;
			str = conf.find(section,name);
			if(str.empty()) {
				std::string err("Conf file error , in section '");
				err += section;
				err = err + "', with index name '";
				err += name;
				err = err + "'";
				throw err;
			}
			return atof(str.c_str());
		}
		
		static	void Load(player_template * pTmp,ONET::Conf & conf, std::string section ,int index)
		{
			ASSERT(index >=0 && index < USER_CLASS_COUNT);
			extend_prop &prop = pTmp->_template_list[index];		
			prop.max_hp 	= ReadInt(conf,section,"hp");
			prop.max_mp	= ReadInt(conf,section,"mp");
			prop.vitality	= ReadInt(conf,section,"vitality");
			prop.energy	= ReadInt(conf,section,"energy");
			prop.strength	= ReadInt(conf,section,"strength");
			prop.agility	= ReadInt(conf,section,"agility");
			prop.attack_speed = ReadInt(conf,section,"attack_speed");
			prop.attack_range = ReadFloat(conf,section,"attack_range");
			prop.walk_speed = ReadFloat(conf,section,"walk_speed");
			prop.run_speed = ReadFloat(conf,section,"run_speed");
			prop.swim_speed = ReadFloat(conf,section,"swim_speed");
			prop.flight_speed = ReadFloat(conf,section,"fly_speed");
			prop.hp_gen	= ReadInt(conf,section,"hp_gen");
			prop.mp_gen	= ReadInt(conf,section,"mp_gen");
			
			player_template::class_data & data = pTmp->_class_list[index];
			data.vit_hp = ReadInt(conf,section, "vit_hp"); 
			data.eng_mp = ReadInt(conf,section, "eng_mp");
			data.lvl_hp = ReadInt(conf,section, "lvlup_hp");
			data.lvl_mp = ReadInt(conf,section, "lvlup_mp");
			data.lvl_dmg = ReadFloat(conf,section, "lvlup_dmg");
			data.lvl_magic = ReadFloat(conf,section, "lvlup_magic");
			data.lvl_defense = ReadFloat(conf,section, "lvlup_defense");
			data.crit_rate = ReadInt(conf,section, "crit_rate");
		}

		static int ReadLevelAdjust(ONET::Conf & conf, std::string section, std::string name,level_adjust & adj)
		{
			std::string str;
			str = conf.find(section,name);
			int level = -1000;
			if(str.empty()) {
				return -1000;
			}
			sscanf(str.c_str(),"%d %f %f %f %f",&level, &adj.exp_adjust, &adj.sp_adjust, &adj.money_adjust, &adj.item_adjust);
			return level;
		}

		static bool ReadA3DVECTORAndRegion(ONET::Conf & conf, std::string section, std::string name,A3DVECTOR &pos, rect  & rt)
		{
			std::string str;
			str = conf.find(section,name);
			if(str.empty()) return false;
			pos = A3DVECTOR(0,0,0);
			rt = rect(0,0,0,0);
			sscanf(str.c_str(),"(%f,%f,%f) (%f,%f,%f,%f)",&pos.x,&pos.y,&pos.z,&rt.left,&rt.top,&rt.right,&rt.bottom);
			return true;
		}

		static	void LoadLevelAdjust(player_template * pTmp,ONET::Conf & conf, std::string section)
		{
			int level;
			level_adjust lad;
			level = ReadLevelAdjust(conf,section,"base_offset",lad);
			if(level != -100)
			{
				throw std::string("can not find baseoffset");
			}
			pTmp->_level_adjust_table[0] = lad;
			int i = 1;
			level = ReadLevelAdjust(conf,section,"offset1",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset2",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset3",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset4",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset5",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset6",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset7",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset8",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"offset9",lad);
			for(;i <=level-BASE_LEVEL_DIFF && i<MAX_LEVEL_DIFF ;i ++){pTmp->_level_adjust_table[i]=lad;}
			level = ReadLevelAdjust(conf,section,"max_offset",lad);
			for(;i < MAX_LEVEL_DIFF;i ++) {pTmp->_level_adjust_table[i]=pTmp->_level_adjust_table[i-1];}
			/*
			for(i = 0; i < MAX_LEVEL_DIFF; i ++)
			{
				__PRINTINFO("%d: %f\n",i + BASE_LEVEL_DIFF,pTmp->_level_adjust_table[i].exp_adjust);
			}*/
		}

		static	void LoadTeamAdjust(player_template * pTmp,ONET::Conf & conf, std::string section)
		{
			for(size_t i =0 ; i < TEAM_MEMBER_CAPACITY+1; i ++)
			{
				char buf[128];
				sprintf(buf,"teamexp%d",i);
				float exp = ReadFloat(conf,section,buf);
				sprintf(buf,"teamsp%d",i);
				float sp = ReadFloat(conf,section,buf);
				pTmp->_team_adjust[i].exp_adjust = exp;
				pTmp->_team_adjust[i].sp_adjust = sp;
			}
		}

		static	void LoadRegion(player_template * pTmp,ONET::Conf & conf, std::string section)
		{
			for(size_t i =0 ; i < MAX_TOWN_REGION; i ++)
			{
				char buf[128];
				sprintf(buf,"region%d",i);
				A3DVECTOR pos;
				rect rt;
				if(!ReadA3DVECTORAndRegion(conf,section,buf,pos,rt)) break;
				pTmp->_region_list.push_back(abase::pair<A3DVECTOR,rect>(pos,rt));
				//__PRINTINFO("region (%f %f)-(%f %f) to pos (%f,%f,%f)\n",rt.left,rt.top,rt.right,rt.bottom,pos.x,pos.y,pos.z);
			}
		}

	};
	ONET::Conf conf(filename);
	bool bRst = true;
	try 
	{
		Data::Load(this,conf,"SWORDSMAN",USER_CLASS_SWORDSMAN);
		Data::Load(this,conf,"MAGE",     USER_CLASS_MAGE);
		Data::Load(this,conf,"NEC",      USER_CLASS_NEC);
		Data::Load(this,conf,"HAG",      USER_CLASS_HAG);
		Data::Load(this,conf,"ORGE",     USER_CLASS_ORGE);
		Data::Load(this,conf,"ASN",      USER_CLASS_ASN);
		Data::Load(this,conf,"ARCHER",   USER_CLASS_ARCHER);
		Data::Load(this,conf,"ANGEL",    USER_CLASS_ANGEL);
		Data::Load(this,conf,"BLADE",	 USER_CLASS_BLADE);
		Data::Load(this,conf,"GENIE",	 USER_CLASS_GENIE);
		Data::Load(this,conf,"SHADOW",	 USER_CLASS_SHADOW);
		Data::Load(this,conf,"FAIRY",	 USER_CLASS_FAIRY);
		Data::LoadLevelAdjust(this,conf,"MESMD_ADJUST");
		Data::LoadTeamAdjust(this,conf,"TEAM_ADJUST");
		Data::LoadRegion(this,conf,"TOWN_REGION");
		_debug_mode = (strcmp(conf.find("GENERAL","debug_command_mode").c_str(),"active") ==0);
		_max_player_level = atoi(conf.find("GENERAL","logic_level_limit").c_str());
		__allow_login_class_mask  = atoi(conf.find("GENERAL","allow_login_class_mask").c_str());
		if(_max_player_level <= 0 || _max_player_level > MAX_PLAYER_LEVEL) _max_player_level = MAX_PLAYER_LEVEL;
		_exp_bonus = atof(conf.find("GENERAL","exp_bonus").c_str());
		_sp_bonus = atof(conf.find("GENERAL","sp_bonus").c_str());
		_money_bonus = atof(conf.find("GENERAL","money_bonus").c_str());
		int p = (strcmp(conf.find("GENERAL","output_debug_info").c_str(),"active") ==0)?1:0;
		__SETPRTFLAG(p);
		p = (strcmp(conf.find("GENERAL","output_normal_info").c_str(),"active") ==0)?1:0;
		__SETPRTFFLAG_INFO(p);

	}
	catch(const std::string & str)
	{
		__PRINTINFO("%s\n",str.c_str());
		return false;
	}

	for(size_t i = 0; i < USER_CLASS_COUNT;i ++)
	{
		extend_prop &prop = _template_list[i];		
		prop.damage_low = 1;
		prop.damage_high = 1;
		prop.damage_magic_low = 1;
		prop.damage_magic_high = 1;
	}
	
	if(bRst == false) return false;
	return __LoadDataFromDataMan(*pDataMan);
}

bool 
player_template::__LoadDataFromDataMan(itemdataman & dataman)
{
	DATA_TYPE  dt;
	size_t id = dataman.get_first_data_id(ID_SPACE_CONFIG,dt);
	int character_flag  = 0;
	for(; id != 0; id = dataman.get_next_data_id(ID_SPACE_CONFIG,dt))
	{	
		if(dt == DT_CHARRACTER_CLASS_CONFIG)
		{
			const CHARRACTER_CLASS_CONFIG &config = *(const CHARRACTER_CLASS_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_CHARRACTER_CLASS_CONFIG && &config);
			int cls = config.character_class_id;
			if(cls < 0 || cls >=USER_CLASS_COUNT)
			{
				ASSERT(false);
				return false;
			}
			if(character_flag & (1 << cls))
			{
				__PRINTINFO("发现了重复的角色数据 为%d号职业\n",cls);
				return false;
			}
			
			_class_list[cls].lvl_hp		= config.lvlup_hp;
			_class_list[cls].lvl_mp		= config.lvlup_mp;
			_class_list[cls].vit_hp		= config.vit_hp;
			_class_list[cls].eng_mp		= config.eng_mp;
			_class_list[cls].lvl_magic	= config.lvlup_magic;
			_class_list[cls].lvl_dmg	= config.lvlup_dmg;
			_class_list[cls].lvl_defense	= config.lvlup_defense;
			_class_list[cls].crit_rate	= config.crit_rate;
			_class_list[cls].faction	= config.faction;
			_class_list[cls].enemy_faction	= config.enemy_faction;
			_class_list[cls].lvl_resistance = config.lvlup_magicdefence;
			_class_list[cls].agi_attack	= config.agi_attack;
			_class_list[cls].agi_armor 	= config.agi_armor;
			_class_list[cls].ap_per_hit 	= config.angro_increase;

			for (size_t i=0; i < PLAYER_FATE_RING_TOTAL; i++)
			{
				_class_list[cls].fatering_adjust[i] = config.spirit_adjust[i];
			}

			_template_list[cls].walk_speed	= config.walk_speed;
			_template_list[cls].run_speed	= config.run_speed;
			_template_list[cls].swim_speed	= config.swim_speed;
			_template_list[cls].flight_speed= config.fly_speed;
			_template_list[cls].attack_speed= (int)(config.attack_speed*20);
			if(_template_list[cls].attack_speed <= 0) _template_list[cls].attack_speed = 30;
			_template_list[cls].attack_range= config.attack_range;
			_template_list[cls].hp_gen 	= config.hp_gen;
			_template_list[cls].mp_gen 	= config.mp_gen;

			_generalcard_cls_adjust_list[cls].reserve(sizeof(config.poker_adjust)/sizeof(config.poker_adjust[0]));
			for(size_t i=0; i<sizeof(config.poker_adjust)/sizeof(config.poker_adjust[0]); i++)
			{
				_generalcard_cls_adjust_list[cls].push_back(config.poker_adjust[i]);
			}
			
			character_flag |= 1 << cls;
		}
		else if(dt == DT_PARAM_ADJUST_CONFIG)
		{
			const PARAM_ADJUST_CONFIG &adjust = *(const PARAM_ADJUST_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_PARAM_ADJUST_CONFIG && &adjust);
			//进行级别差修正
			int level = 100;
			int j = MAX_LEVEL_DIFF;
			level_adjust lad = {0.f,0.f,0.f,0.f};
			for(size_t i =0; i < 16; i++)
			{
				if(adjust.level_diff_adjust[i].level_diff == 0) break;
				if(adjust.level_diff_adjust[i].level_diff >= level ) 
				{
					__PRINTINFO("在级别经验物品修正表内发现错误\n");
					return false;
				}
				level = adjust.level_diff_adjust[i].level_diff;
				lad.exp_adjust		= adjust.level_diff_adjust[i].adjust_exp;
				lad.sp_adjust		= adjust.level_diff_adjust[i].adjust_sp;
				lad.money_adjust	= adjust.level_diff_adjust[i].adjust_money;	
				lad.item_adjust		= adjust.level_diff_adjust[i].adjust_matter;
				lad.attack_adjust 	= adjust.level_diff_adjust[i].adjust_attack;
				if(lad.attack_adjust < 0.f) lad.attack_adjust = 0.f;
				if(lad.attack_adjust > 1.f) lad.attack_adjust = 0.f;
				for(;j >= level-BASE_LEVEL_DIFF && j >= 0;j-- ){_level_adjust_table[j]=lad;}
			}
			for(;j >= 0 ;j-- ) {_level_adjust_table[j]=lad;}

			//进行队伍修正
			for(size_t i = 0; i < TEAM_MEMBER_CAPACITY+1 ; i ++)
			{	
				_team_adjust[i].exp_adjust	= adjust.team_adjust[i].adjust_exp;
				_team_adjust[i].sp_adjust	= adjust.team_adjust[i].adjust_sp;
			}

			for(size_t i =0; i < USER_CLASS_COUNT+1; i ++)
			{
				_team_race_adjust[i].exp_adjust	= adjust.team_profession_adjust[i].adjust_exp/20.0f;
				_team_race_adjust[i].sp_adjust	= adjust.team_profession_adjust[i].adjust_sp/20.f;
				//_team_race_adjust[i].exp_adjust	= adjust.team_profession_adjust[i].adjust_exp;
				//_team_race_adjust[i].sp_adjust	= adjust.team_profession_adjust[i].adjust_sp;
			}
			
			for(size_t i = 0; i < PLAYER_FATE_RING_MAX_LEVEL; i++)
			{
				_use_soul_exp_adjust[i] = adjust.use_monster_spirit_adjust[i];
			}

			dmg_adj_to_spec_atk_speed = adjust.dmg_adj_to_spec_atk_speed;
			if(dmg_adj_to_spec_atk_speed > 1.f || dmg_adj_to_spec_atk_speed < 0.1f) dmg_adj_to_spec_atk_speed = 1.f;
			atk_rate_adj_to_spec_atk_speed = adjust.atk_rate_adj_to_spec_atk_speed;
			if(atk_rate_adj_to_spec_atk_speed > 1.f || atk_rate_adj_to_spec_atk_speed < 0.1f) atk_rate_adj_to_spec_atk_speed = 1.f;

		}
		else if(dt == DT_PLAYER_LEVELEXP_CONFIG)
		{
			const PLAYER_LEVELEXP_CONFIG  &level= *(const PLAYER_LEVELEXP_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_PLAYER_LEVELEXP_CONFIG && &level);

			if(id == 202)
			{
				//玩家省级曲线表
				for(size_t i = 1; i <= (size_t)GetMaxLevelLimit(); i ++)
				{
					if(level.exp[i-1])
					{
						_exp_list[i] = level.exp[i-1];
					}
					else
					{
						__PRINTINFO("级别%d升级所需经验值为0\n",i);

					}
				}
			}
			else if(id == 592)
			{
				//宠物省级曲线表
				for(size_t i = 1; i <= (size_t)GetMaxLevelLimit(); i ++)
				{
					if(level.exp[i-1])
					{
						_pet_exp_list[i] = level.exp[i-1];
					}
					else
					{
						__PRINTINFO("宠物级别%d升级所需经验值为0\n",i);

					}
				}
			}
		}
		else if(dt == DT_PLAYER_SECONDLEVEL_CONFIG)
		{
			const PLAYER_SECONDLEVEL_CONFIG &exp= *(const PLAYER_SECONDLEVEL_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_PLAYER_SECONDLEVEL_CONFIG && &exp);

			for(size_t i = 0; i < 256; i ++)
			{
				if(exp.exp_lost[i])
				{
					_death_exp_punish[i] = exp.exp_lost[i];
				}
				if(exp.exp_lost[i] > 0.05001)
				{
					__PRINTINFO("发现死亡经验损失大于5%, %d\n",i);
				}
			}
		}
		else if(dt == DT_PLAYER_REALM_CONFIG)
		{
			const PLAYER_REALM_CONFIG &exp = *(const PLAYER_REALM_CONFIG*)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_PLAYER_REALM_CONFIG && &exp );
			ASSERT(sizeof(exp.list) == MAX_REALM_LEVEL*8 ); // for 10*10*2 int 

			_realm_list.push_back(realm_node(0,0)); // 0 for default

			for(int n = 0; n < 10; ++n)
			{
				for(int m = 0; m < 10; ++m)
				{
					_realm_list.push_back(realm_node(exp.list[n].level[m].require_exp,
								exp.list[n].level[m].vigour));	
				}
			}
		}
		else if(dt == DT_POKER_LEVELEXP_CONFIG)
		{
			const POKER_LEVELEXP_CONFIG & config = *(const POKER_LEVELEXP_CONFIG *)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_POKER_LEVELEXP_CONFIG && &config);

			//ElementData升级经验数组的索引0对应于1级升级经验,故插入一个占位元素
			_generalcard_exp_list.push_back(0);
			for(size_t i=0; i<sizeof(config.exp)/sizeof(config.exp[0]); i++)
			{
				_generalcard_exp_list.push_back(config.exp[i]);
			}

			for(size_t i=0; i<sizeof(config.exp_adjust)/sizeof(config.exp_adjust[0]); i++)
			{
				_generalcard_exp_rank_adjust_list.push_back(config.exp_adjust[i]);
			}
		}
		else if(dt == DT_ASTROLABE_LEVELEXP_CONFIG)
		{
			const ASTROLABE_LEVELEXP_CONFIG & config = *(const ASTROLABE_LEVELEXP_CONFIG *)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_ASTROLABE_LEVELEXP_CONFIG && &config);

			int totalexp = 0;
			for(size_t i =0; i<sizeof(config.exp)/sizeof(config.exp[0]); i++)
			{				
				_astrolabe_total_exp_list.push_back(totalexp);
				_astrolabe_levelup_exp_list.push_back(config.exp[i]);
				totalexp += config.exp[i];
			}
		}
		else if(dt == DT_ASTROLABE_ADDON_RANDOM_CONFIG)
		{
			const ASTROLABE_ADDON_RANDOM_CONFIG & config = *(const ASTROLABE_ADDON_RANDOM_CONFIG *)dataman.get_data_ptr(id,ID_SPACE_CONFIG,dt);
			ASSERT(dt == DT_ASTROLABE_ADDON_RANDOM_CONFIG && &config);

			for(size_t i =0; i<sizeof(config.levelup_exp)/sizeof(config.levelup_exp[0]); i++)
			{
				_astrolabe_vip_levelup_exp_list.push_back(config.levelup_exp[i]);
			}
			
			for(size_t i =0; i<sizeof(config.rand_probability)/sizeof(config.rand_probability[0]); i++)
			{
				_astrolabe_addon_prob_list.push_back(astrolabe_addon_prob(config.rand_probability[i]));	
			}
		}
	}

	if(character_flag != ((1<< USER_CLASS_COUNT) -1))
	{
		__PRINTINFO("模板里缺少某些角色数据 角色组合代码为：%08x\n",character_flag);
		return false;
	}
	return true;
}

bool 
player_template::__CopyTemplate(int cls, extend_prop & data) const
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return false; 
	memcpy(&data,&_template_list[cls],sizeof(extend_prop));
	return true;
}

bool 
player_template::__LevelUp(int cls, int oldlvl,extend_prop & data) const
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return false; 
	if(oldlvl <=0 || oldlvl > 10000) return false;
	data.max_hp += _class_list[cls].lvl_hp;
	data.max_mp +=_class_list[cls].lvl_mp;
	int inc1 = (int)(oldlvl * _class_list[cls].lvl_dmg);
	int inc = (int)((oldlvl+1) * _class_list[cls].lvl_dmg);
	inc -= inc1;
	data.damage_low += inc;
	data.damage_high += inc;

	inc1 = (int)(oldlvl * _class_list[cls].lvl_magic);
	inc = (int)((oldlvl+1) * _class_list[cls].lvl_magic);
	inc -= inc1;
	data.damage_magic_low += inc;
	data.damage_magic_high += inc;

	inc1 = (int)(oldlvl * _class_list[cls].lvl_defense);
	inc = (int)((oldlvl+1) * _class_list[cls].lvl_defense);
	data.defense += inc - inc1;

	inc1 = (int)(oldlvl * _class_list[cls].lvl_resistance);
	inc = (int)((oldlvl+1) * _class_list[cls].lvl_resistance);
	inc -= inc1;
	data.resistance[0] += inc;
	data.resistance[1] += inc;
	data.resistance[2] += inc;
	data.resistance[3] += inc;
	data.resistance[4] += inc;
	return true;
}

bool 
player_template::__LevelRollback(int cls, int oldlvl,extend_prop & data) const
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return false; 
	if(oldlvl <=0 || oldlvl > 10000) return false;
	if(oldlvl == 1) return true;

	data.max_hp += _class_list[cls].lvl_hp * (1 - oldlvl);
	data.max_mp += _class_list[cls].lvl_mp * (1 - oldlvl);
	int inc1 = (int)(oldlvl * _class_list[cls].lvl_dmg);
	int inc = (int)(1 * _class_list[cls].lvl_dmg);
	inc -= inc1;
	data.damage_low += inc;
	data.damage_high += inc;

	inc1 = (int)(oldlvl * _class_list[cls].lvl_magic);
	inc = (int)(1 * _class_list[cls].lvl_magic);
	inc -= inc1;
	data.damage_magic_low += inc;
	data.damage_magic_high += inc;

	inc1 = (int)(oldlvl * _class_list[cls].lvl_defense);
	inc = (int)(1 * _class_list[cls].lvl_defense);
	data.defense += inc - inc1;

	inc1 = (int)(oldlvl * _class_list[cls].lvl_resistance);
	inc = (int)(1 * _class_list[cls].lvl_resistance);
	inc -= inc1;
	data.resistance[0] += inc;
	data.resistance[1] += inc;
	data.resistance[2] += inc;
	data.resistance[3] += inc;
	data.resistance[4] += inc;
	return true;
}

bool 
player_template::__UpdateBasic(int cls, extend_prop & data, int vit,int eng,int str,int agi) const
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return false; 
	data.vitality += vit;
	data.energy += eng;
	data.strength += str;
	data.agility += agi;
	
	data.max_hp += _class_list[cls].vit_hp * vit;
	data.max_mp += _class_list[cls].eng_mp * eng;
	return true;
}

int  
player_template::__GetVitHP(int cls, int vit)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return 0; 
	return _class_list[cls].vit_hp * vit;
}

int  
player_template::__GetEngMP(int cls, int eng)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return 0; 
	return _class_list[cls].eng_mp * eng;
}

int  
player_template::__Rollback(int cls, extend_prop & data) const
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return 0; 
	int total = 0;
	int vit,eng,str,agi;
	vit = 5 - data.vitality;
	eng = 5 - data.energy;
	str = 5 - data.strength;
	agi = 5 - data.agility;
	if(str >0 ) str = 0;
	if(agi >0 ) agi = 0;
	if(vit >0 ) vit = 0;
	if(eng >0 ) eng = 0;
	UpdateBasic(cls,data,vit,eng,str,agi);
	total =-(vit + eng + str + agi);
	return total;
}

int  
player_template::__Rollback(int cls, extend_prop & data,int str, int agi ,int vit, int eng) const
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return 0; 
	int total = 0;
	int vit1,eng1,str1,agi1;
	vit1 = 5 - data.vitality;	//3->5, fix bug by liuguichen, 20130721
	eng1 = 5 - data.energy;		//由于bug存在,部分玩家可能体力灵力小于5
	str1 = 5 - data.strength;
	agi1 = 5 - data.agility;
	vit = -vit; str = -str; agi = -agi; eng = -eng;
	
	if(str < str1) str = str1;
	if(agi < agi1) agi = agi1;
	if(vit < vit1) vit = vit1;
	if(eng < eng1) eng = eng1;
	if(str >0 ) str = 0;
	if(agi >0 ) agi = 0;
	if(vit >0 ) vit = 0;
	if(eng >0 ) eng = 0;
	
	UpdateBasic(cls,data,vit,eng,str,agi);
	total = -(vit + eng + str + agi);
	return total;
}

int  
player_template::__GetLvlupExp(int cls, int cur_lvl) const
{
	if(((size_t)cur_lvl) <= (size_t)GetMaxLevelLimit())
	{
		return _exp_list[cur_lvl];
	}
	return cur_lvl * cur_lvl * 500;
}

int  
player_template::__GetPetLvlupExp(int cur_lvl) const
{
	if(((size_t)cur_lvl) <= (size_t)GetMaxLevelLimit())
	{
		return _pet_exp_list[cur_lvl];
	}
	return cur_lvl * cur_lvl * 500;
}

void 
player_template::__InitPlayerData(int cls,gactive_imp * pImp)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) 
	{
		ASSERT(false);
		return; 
	}
	pImp->_crit_rate = _class_list[cls].crit_rate;
	const extend_prop & prop = _template_list[cls];
	pImp->_base_prop.attack_range = prop.attack_range;
	pImp->_base_prop.attack_speed = prop.attack_speed;
	pImp->_base_prop.walk_speed = prop.walk_speed;
	pImp->_base_prop.run_speed = prop.run_speed;
	pImp->_base_prop.flight_speed = prop.flight_speed;
	pImp->_base_prop.swim_speed = prop.swim_speed;
	pImp->_base_prop.hp_gen = prop.hp_gen;
	pImp->_base_prop.mp_gen = prop.mp_gen;
	pImp->_faction = _class_list[cls].faction;
	pImp->_enemy_faction = _class_list[cls].enemy_faction;
	pImp->SetPerHitAP(_class_list[cls].ap_per_hit);
}

bool
player_template::__GetTownPosition(const A3DVECTOR & source, A3DVECTOR & target)
{
	for(size_t i = 0; i < _region_list.size(); i ++)
	{
		if(_region_list[i].second.IsIn(source.x,source.z))
		{
			target = _region_list[i].first;
			return true;
		}
	}
	return false;
}

int  
player_template::__GetBasicAttackRate(int cls,int agility)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return agility; 
	return _class_list[cls].agi_attack * agility;
}

int  
player_template::__GetBasicArmor(int cls,int agility)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return agility; 
	return _class_list[cls].agi_armor * agility;
}

float 
player_template::__GetResurrectExpReduce(size_t sec_level)
{
	return _death_exp_punish[sec_level & 0xFF];
}

float
player_template::__GetFateRingAdjust(int cls, int index)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return 0.f; 
	if(((size_t)index) >= PLAYER_FATE_RING_TOTAL) return 0.f;
	return _class_list[cls].fatering_adjust[index];
}

float
player_template::__GetUseSoulExpAdjust(int fatering_level, int soul_level)
{
	int level_sub = soul_level - fatering_level;
	if (level_sub > PLAYER_FATE_RING_MAX_LEVEL || level_sub < - PLAYER_FATE_RING_MAX_LEVEL)
		return 0.f;
	else if (level_sub <= 0)	
		return 1.f;	//使用同级和低级的元魂没有衰减

	return _use_soul_exp_adjust[level_sub-1];
}

#define	MAX_BASE_HP  	400
#define	MAX_BASE_MP 	400
#define MAX_BASE_DAMAGE	50
#define MAX_BASE_DEFENSE 155
#define MAX_BASE_ARMOR 	50

bool 
player_template::__CheckData(int cls, int level, const extend_prop &data)
{
	if(((size_t)cls) >= USER_CLASS_COUNT) return false; 
	//检查血量
	int max_hp = _class_list[cls].lvl_hp * (level - 1) + MAX_BASE_HP + _class_list[cls].vit_hp * data.vitality;
	int max_mp = _class_list[cls].lvl_mp * (level - 1) + MAX_BASE_MP + _class_list[cls].eng_mp * data.energy;

	if(max_hp < data.max_hp)
	{
		return false;
	}

	if(max_mp < data.max_mp)
	{
		return false;
	}

	//检查物理攻击
	int dmg = (int)(level * _class_list[cls].lvl_dmg) + MAX_BASE_DAMAGE;
	if(dmg < data.damage_high) 
	{
		return false;
	}
	
	//检查法术攻击
	int magic_damage = (int)(level * _class_list[cls].lvl_magic) + MAX_BASE_DAMAGE;
	if(magic_damage < data.damage_magic_high)
	{
		return false;
	}

	//检查五系防御
	int res = (int)(level * _class_list[cls].lvl_resistance) + MAX_BASE_DEFENSE;
	if( data.resistance[0] > res || data.resistance[1] > res || data.resistance[2] > res || data.resistance[3] > res || data.resistance[4] > res)
	{
		return false;
	}

	//检查防御
	int defense = (int)(level * _class_list[cls].lvl_defense) + MAX_BASE_DEFENSE;
	if(data.defense > defense)
	{
		return false;
	}

	//检查闪躲
	if(data.armor > MAX_BASE_ARMOR)
	{
		return false;
	}
	return true;
}

int 
player_template::__GetRealmLvlupExp(int realmlvl) const
{
	if ( realmlvl < 0 || realmlvl > MAX_REALM_LEVEL ) return INT_MAX;
	return _realm_list[realmlvl].exp_goal;
}

int 
player_template::__GetRealmVigour(int realmlvl) const
{
	if ( realmlvl < 0 || realmlvl > MAX_REALM_LEVEL ) return 0;
	return _realm_list[realmlvl].vigour_base;
	
}

float 
player_template::__GetGeneralCardClsAdjust(int cls, int card_type)
{
	if((size_t)cls >= USER_CLASS_COUNT) return 0.f;
	if((size_t)card_type >= _generalcard_cls_adjust_list[cls].size()) return 0.f;
	return _generalcard_cls_adjust_list[cls][card_type];
}

int 
player_template::__GetGeneralCardLvlupExp(int rank, int lvl)
{
	if((size_t)rank >= _generalcard_exp_rank_adjust_list.size()) return INT_MAX;
	if((size_t)lvl >= _generalcard_exp_list.size()) return INT_MAX;
	return (int)(_generalcard_exp_list[lvl] * _generalcard_exp_rank_adjust_list[rank] + 0.5f);
}

int 
player_template::__GetAstrolabeAddonCount(int vip)
{
	if((size_t) vip >= _astrolabe_addon_prob_list.size() )	return 0;
	astrolabe_addon_prob& probs = _astrolabe_addon_prob_list[vip];
//	int index = abase::RandSelect(probs.probability,ASTROLABE_ADDON_MAX);
	int index = 0;
	for(; index < ASTROLABE_ADDON_MAX; ++index)
	{
		if(probs.probability[index] >= 1.f)
			continue;
		else if(probs.probability[index] < abase::RandUniform())
			break;
	}

	return index;
}

int
player_template::__GetAstrolabeVipGradeExp(int vlevel)
{
	if((size_t)vlevel > _astrolabe_vip_levelup_exp_list.size()) return INT_MAX;
	return _astrolabe_vip_levelup_exp_list[vlevel];
}

int
player_template::__GetAstrolabeLvlupExp(int lvl)
{
	if((size_t)lvl > _astrolabe_levelup_exp_list.size()) return INT_MAX;
	return _astrolabe_levelup_exp_list[lvl];
}

int
player_template::__GetAstrolabeLvltotalExp(int lvl)
{
	if((size_t)lvl > _astrolabe_total_exp_list.size()) return 0;
	return _astrolabe_total_exp_list[lvl];
}

