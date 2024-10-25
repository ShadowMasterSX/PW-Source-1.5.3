#include "autoteamman.h"
#include "template/itemdataman.h"
#include "template/globaldataman.h"
#include <common/protocol.h>
#include <gsp_if.h>
#include "world.h"
#include "player_imp.h"

bool autoteam_man::InitAutoTeamConfig(itemdataman& dataman)
{
	//从elementdata初始化数据
	DATA_TYPE dt;
	for(size_t id = dataman.get_first_data_id(ID_SPACE_CONFIG, dt); id != 0; id = dataman.get_next_data_id(ID_SPACE_CONFIG,dt)) {
		if(dt == DT_AUTOTEAM_CONFIG) {
			const AUTOTEAM_CONFIG &config = *(const AUTOTEAM_CONFIG*)dataman.get_data_ptr(id, ID_SPACE_CONFIG, dt); 
			ASSERT(dt == DT_AUTOTEAM_CONFIG);
			ASSERT((sizeof(config.require_level) / sizeof(config.require_level[0])) == 2);
			ASSERT((sizeof(config.require_num) / sizeof(config.require_num[0])) == 2);

			autoteam_goal goal;
			goal._id = config.id;
			goal._require_lvl[0] = config.require_level[0];
			goal._require_lvl[1] = config.require_level[1];
			goal._require_maxlvl[0] = config.require_maxlevel[0];
			goal._require_maxlvl[1] = config.require_maxlevel[1];
			goal._require_gender = config.require_gender;
			goal._need_player_cnt = config.require_num[0];
			goal._sec_lvl = config.require_level2;
			goal._require_reincarnation_times[0] = config.require_reincarnation_times[0];
			goal._require_reincarnation_times[1] = config.require_reincarnation_times[1];
			goal._require_realm_lvl[0] = config.require_realm_level[0];
			goal._require_realm_lvl[1] = config.require_realm_level[1];
			goal._soul_floor = config.require_soul_power;
			goal._worldtag = config.worldtag;
			goal._entrance_pos.x = config.trans_pos[0];
			goal._entrance_pos.y = config.trans_pos[1];
			goal._entrance_pos.z = config.trans_pos[2];
			
			for(unsigned int i = 0; i < 32; ++i) {
				if(config.worldtag_from[i] <= 0) break;
				goal._worldtag_from_list.push_back(config.worldtag_from[i]);
			}
			
			char occupation_need_cnt = 0;
			for(unsigned int i = 0; i < USER_CLASS_COUNT; ++i) {
				if(config.num_prof[i] > 0) { 	
					OCCUPATION_NEED info;
					info.first = i;
					info.second = config.num_prof[i];
					goal._occupation_config_list.push_back(info);

					occupation_need_cnt += info.second;
				}
			}
			
			if(goal._need_player_cnt < 2 || goal._need_player_cnt > TEAM_MEMBER_CAPACITY) return false;
			if(occupation_need_cnt > goal._need_player_cnt) return false;

			_goal_map[goal._id] = goal;
		}
	}
	
	if(_goal_map.empty()) return false;
	return true;
}

void autoteam_man::SendConfigData()
{
	unsigned int goal_cnt = (unsigned int)_goal_map.size();
	int config_size = sizeof(GMSV::AutoTeamConfig) + goal_cnt * sizeof(GMSV::AutoTeamConfig::Goal);
	GMSV::AutoTeamConfig* pConfig = (GMSV::AutoTeamConfig*)abase::fastalloc(config_size);
	pConfig->goal_cnt = goal_cnt;
	
	int goal_idx = 0;
	for(GOAL_CONFIG_MAP::iterator it = _goal_map.begin(); it != _goal_map.end(); ++it) {
		autoteam_goal& goal = it->second;
		GMSV::AutoTeamConfig::Goal& info = pConfig->goal_list[goal_idx];
		 
		info.id = goal._id;
		info.need_player_cnt = goal._need_player_cnt;
		unsigned int list_size = goal._occupation_config_list.size();
		info.occupation_list_size = list_size;
		
		if(list_size != 0) {
			info.occupation_list = (GMSV::AutoTeamConfig::OccupationInfo*)abase::fastalloc(sizeof(GMSV::AutoTeamConfig::OccupationInfo) * list_size); 

			for(unsigned int i = 0; i < list_size; ++i) {
				info.occupation_list[i].occupation = goal._occupation_config_list[i].first;
				info.occupation_list[i].need_cnt = goal._occupation_config_list[i].second;
			}
		} else {
			info.occupation_list = NULL;
		}

		++goal_idx;
	}

	GMSV::SendAutoTeamData(pConfig);
	
	for(unsigned int i = 0; i < goal_cnt; ++i) {
		GMSV::AutoTeamConfig::Goal& info = pConfig->goal_list[i];
		if(info.occupation_list != NULL) {
			abase::fastfree(info.occupation_list, sizeof(GMSV::AutoTeamConfig::OccupationInfo) * info.occupation_list_size);
			info.occupation_list = NULL;
		}
	}
	
	abase::fastfree(pConfig, config_size);
}

bool autoteam_man::CanPlayerAutoComposeTeam(int goal_id, gplayer_imp* pImp)
{		
	GOAL_CONFIG_MAP::iterator it = _goal_map.find(goal_id);
	if(it == _goal_map.end()) return false;
	autoteam_goal& goal = it->second;
	
	int gender = pImp->IsPlayerFemale() ? 1 : 0;
	if(goal._require_gender != 2 && gender != goal._require_gender) return false; //性别限制 （0 男、1 女、2 不限制）
	
	if( (pImp->_basic.level < goal._require_lvl[0]) || ((goal._require_lvl[1] != 0) && (pImp->_basic.level > goal._require_lvl[1])) ) return false;//玩家最低/最高等级（0 为不限制）
	if( (pImp->GetHistoricalMaxLevel() < goal._require_maxlvl[0]) || ((goal._require_maxlvl[1] != 0) && (pImp->GetHistoricalMaxLevel() > goal._require_maxlvl[1])) ) return false;
	if( pImp->_basic.sec_level < goal._sec_lvl ) return false;
	if( ((int)pImp->GetReincarnationTimes() < goal._require_reincarnation_times[0])
		|| ((goal._require_reincarnation_times[1] != 0) && ((int)pImp->GetReincarnationTimes() > goal._require_reincarnation_times[1])) ) return false;
	if( (pImp->GetRealmLevel() < goal._require_realm_lvl[0]) || ((goal._require_realm_lvl[1] != 0) && (pImp->GetRealmLevel() > goal._require_realm_lvl[1])) ) return false;
	if(pImp->GetSoulPower() < goal._soul_floor) return false;
	
	return true;
}

bool autoteam_man::GetGoalEntrancePos(int goal_id, gplayer_imp* pImp, int current_worldtag, int& target_tag, A3DVECTOR& target_pos)
{
	GOAL_CONFIG_MAP::iterator it = _goal_map.find(goal_id);
	if(it == _goal_map.end()) return false;
	autoteam_goal& goal = it->second;
	
	bool can_jump = false;
	for(unsigned int i = 0; i < goal._worldtag_from_list.size(); ++i) {
		if(current_worldtag == goal._worldtag_from_list[i]) {
			can_jump = true;
			break;
		}
	}
	if(!can_jump) return false;
	if( (pImp->_basic.level < goal._require_lvl[0]) || ((goal._require_lvl[1] != 0) && (pImp->_basic.level > goal._require_lvl[1])) ) return false;//玩家最低/最高等级（0 为不限制）
	if( (pImp->GetHistoricalMaxLevel() < goal._require_maxlvl[0]) || ((goal._require_maxlvl[1] != 0) && (pImp->GetHistoricalMaxLevel() > goal._require_maxlvl[1])) ) return false;
	if( pImp->_basic.sec_level < goal._sec_lvl ) return false;
	if( ((int)pImp->GetReincarnationTimes() < goal._require_reincarnation_times[0])
		|| ((goal._require_reincarnation_times[1] != 0) && ((int)pImp->GetReincarnationTimes() > goal._require_reincarnation_times[1])) ) return false;
	if( (pImp->GetRealmLevel() < goal._require_realm_lvl[0]) || ((goal._require_realm_lvl[1] != 0) && (pImp->GetRealmLevel() > goal._require_realm_lvl[1])) ) return false;

	target_tag = goal._worldtag;
	target_pos = goal._entrance_pos;

	return true;
}

