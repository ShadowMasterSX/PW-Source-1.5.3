#include "playersolochallenge.h"
#include "world.h"
#include "player_imp.h"
#include "task/taskman.h"
#include "cooldowncfg.h"
#include "aei_filter.h"


void playersolochallenge::NotifySoloChallengeData(gplayer_imp *pImp)
{	
	int last_success_stage_level     = 0;
	int last_success_stage_cost_time = 0;
	int draw_award_times             = 0;
	if(_left_draw_award_times || award_info.size())
	{
		last_success_stage_level     = _max_stage_level;
		last_success_stage_cost_time = _max_stage_cost_time;
		draw_award_times             = _left_draw_award_times + award_info.size();
	}
	pImp->_runner->solo_challenge_award_info_notify(_max_stage_level, _total_first_climbing_time, _total_score, _cur_score, last_success_stage_level, last_success_stage_cost_time, draw_award_times, award_info.size(), award_info);
}

void playersolochallenge::PlayerEnterSoloChallengeInstance(gplayer_imp *pImp)
{
	_cur_score = _total_score;
	int filter_index[ELEMENTDATA_NUM_SOLO_TOWER_CHALLENGE_BUFF_COUNT] = {0};
	for(int i = 0; i < ELEMENTDATA_NUM_SOLO_TOWER_CHALLENGE_BUFF_COUNT; i++)
	{
		filter_index[i] = i;
	}
	int filter_num[ELEMENTDATA_NUM_SOLO_TOWER_CHALLENGE_BUFF_COUNT] = {0};
	pImp->_runner->solo_challenge_buff_info_notify(filter_index, filter_num, ELEMENTDATA_NUM_SOLO_TOWER_CHALLENGE_BUFF_COUNT, _cur_score);
	//pImp->_filters.Clear();//大bug,别用这个函数哈
	pImp->_filters.ClearSpecFilter(filter::FILTER_MASK_DEBUFF | filter::FILTER_MASK_BUFF);

	pImp->_filters.AddFilter(new aesl_filter(pImp,FILTER_CHECK_INSTANCE_KEY));
	
	GLog::formatlog("SOLO_CHALLENGE_LOG ENTER 用户 %d 进入单人副本.",pImp->_parent->ID.id);
}

void playersolochallenge::PlayerLeaveSoloChallengeInstance(gplayer_imp *pImp)
{
	GLog::formatlog("SOLO_CHALLENGE_LOG LEAVE 用户 %d 离开单人副本.",pImp->_parent->ID.id);
}

void playersolochallenge::CalcPlayMode(SOLO_TOWER_CHALLENGE_LEVEL_CONFIG *conf, int stage_level, gplayer_imp *pImp)
{
	int playmode_count = 0;
	for(int i = 0; i < 32; ++i)
	{
		if(conf -> steps[(stage_level-1)/9].playing_method_controller[i] == 0)
		  break;
		++playmode_count;
	}//从配置表得到阶段的玩法数
	
	if(stage_level % 9 == 0)//9的倍数关
	{
		int play_mode_controller = conf->steps[(stage_level-1)/9].playing_method_boss_controller;
		SET_CV(CUR_STAGE_PLAY_MODE,play_mode_controller);
		pImp->_plane->TriggerSpawn(play_mode_controller);
		return;
	}

	if((_max_stage_level - 1) / 9 > (stage_level - 1) / 9)//选择以前阶段的关卡,随机选择一种玩法
	{
		int index = abase::Rand(0,playmode_count-1);
		
		int play_mode_controller = conf->steps[(stage_level-1)/9].playing_method_controller[index];
		SET_CV(CUR_STAGE_PLAY_MODE,play_mode_controller);
		pImp->_plane->TriggerSpawn(play_mode_controller);
		return;
	}
	if(IsTopLevel(stage_level))//未玩过的关卡保证玩法不重复
	{
		if(stage_level % 9 == 1)
		{
			_play_modes = 0;
		}

		int shift = 0;
		std::vector<int> left_play_mode;
		while(shift < playmode_count)
		{
			if(!((1 << shift) & _play_modes))
			  left_play_mode.push_back(shift);
			shift++;
		}
	
		if(left_play_mode.size() == 0)
		{
			SET_CV(CUR_STAGE_PLAY_MODE,-1);
			return;
		}
		int index = abase::Rand(0,left_play_mode.size() - 1);//进行玩法的随机
		
		int play_mode_controller = conf->steps[(stage_level-1)/9].playing_method_controller[ left_play_mode[index] ];
		_cur_stage_play_mode = left_play_mode[index];
		SET_CV(CUR_STAGE_PLAY_MODE,play_mode_controller);
		pImp->_plane->TriggerSpawn(play_mode_controller);
	}
	else//已玩过的关卡随机选取已经玩过的玩法
	{
		if(_max_stage_level % 9 == 0 && _play_modes == 0)
		{
			_play_modes = (1 << playmode_count) - 1;
		}
		std::vector<int> have_play_mode;
		int shift = 0;
		while(shift < playmode_count)
		{
			if(((1 << shift) & _play_modes))
			  have_play_mode.push_back(shift);
			shift++;
		}
	
		if(have_play_mode.size() == 0)
		{
			SET_CV(CUR_STAGE_PLAY_MODE,-1);
			return;
		}
		
		int index = abase::Rand(0,have_play_mode.size() - 1);//进行玩法的随机
		int play_mode_controller = conf->steps[(stage_level-1)/9].playing_method_controller[ have_play_mode[index] ];
		SET_CV(CUR_STAGE_PLAY_MODE,play_mode_controller);
		pImp->_plane->TriggerSpawn(play_mode_controller);
	}
}

void playersolochallenge::SelectStage(gplayer_imp *pImp,int stage_level)
{
 	ASSERT((stage_level > 0) && (stage_level <= SOLO_TOWER_CHALLENGE_MAX_STAGE));
 	if( (stage_level > (_max_stage_level + 1)) || (stage_level > SOLO_TOWER_CHALLENGE_MAX_STAGE) )
 	{
		pImp->_runner->solo_challenge_operate_result(C2S:: SOLO_CHALLENGE_OPT_SELECT_STAGE, S2C::ERR_SOLO_CHALLENGE_TOP_STAGE, stage_level, _max_stage_level,0);
 		return;
 	}

 	DATA_TYPE dt;
 	SOLO_TOWER_CHALLENGE_LEVEL_CONFIG* conf = (SOLO_TOWER_CHALLENGE_LEVEL_CONFIG*)world_manager::GetDataMan().get_data_ptr(SOLO_TOWER_CHALLENGE_LEVEL_CONFIG_ID, ID_SPACE_CONFIG, dt);
 	if ((conf == NULL) || (dt != DT_SOLO_TOWER_CHALLENGE_LEVEL_CONFIG))
 	{
		pImp->_runner->solo_challenge_operate_result(C2S:: SOLO_CHALLENGE_OPT_SELECT_STAGE, S2C::ERR_SOLO_CHALLENGE_FAILURE, stage_level, _max_stage_level,0);
 		return;
 	}
 
 	int index = (stage_level - 1) / SOLO_TOWER_CHALLENGE_STAGE_EVERYROOM;
 
 	float * pos = conf->room[index].trans_pos;
 	A3DVECTOR trans_pos(pos[0], pos[1], pos[2]);
	
 	if(!pImp->_plane->PosInWorld(trans_pos))
 	{
		pImp->_runner->solo_challenge_operate_result(C2S:: SOLO_CHALLENGE_OPT_SELECT_STAGE, S2C::ERR_SOLO_CHALLENGE_FAILURE, stage_level, _max_stage_level,0);
 		return;
 	}

	PlayerTaskInterface  task_if(pImp);
	ClearAllTowerTask(&task_if);//清除所有单人副本任务
	award_info.clear();//进入下一关就清楚奖励信息
	_left_draw_award_times  = 0;
	_max_stage_cost_time = 0;
	
	
 	pImp->LongJump(trans_pos);
 	
 	for(int i = 0;i < 256;i++)
 	{
		if(conf->controller_id_to_deactivate[i] == 0)
			break;
		for(int j = 0; j < SOLO_TOWER_CHALLENGE_MAX_STAGE / SOLO_TOWER_CHALLENGE_STAGE_EVERYROOM; ++j)
		{
			pImp->_plane->ClearSpawn( npc_generator::GenBlockUniqueID(conf->controller_id_to_deactivate[i], j) );
		}
 	}//关闭控制器
 	for(int i = 0;i < 8;i++)
 	{
		if(conf->room[index].controller_id_to_activate[i] == 0)
			break;
 		pImp->_plane->TriggerSpawn(conf->room[index].controller_id_to_activate[i]);
 	}//开启控制器

	SET_CV(CUR_STAGE_LEVEL,stage_level);
	SET_CV(CUR_STAGE_STATE,STAGE_WAIT_START);
	CalcPlayMode(conf, stage_level, pImp);//计算玩法
	pImp->_runner->solo_challenge_operate_result(C2S:: SOLO_CHALLENGE_OPT_SELECT_STAGE, S2C::ERR_SUCCESS, stage_level, _max_stage_level,0);
	
	GLog::log(GLOG_INFO,"SOLO_CHALLENGE_LOG SELECT_SATGE 用户 %d 选择第 %d 关,时间 %ld, 玩法 %d, 玩法记录 %d.",pImp->_parent->ID.id, stage_level, g_timer.get_systime(), _cur_stage_play_mode, _play_modes);
}

void playersolochallenge::StageComplete(gplayer_imp *pImp, bool isCompleteSuccess)//单人副本发奖
{
	int cur_stage_level = GET_CV(CUR_STAGE_LEVEL);

	int stage_state = GET_CV(CUR_STAGE_STATE);
	if(stage_state != STAGE_PROCESSING || !isCompleteSuccess)
	{
		pImp->_runner->solo_challenge_challenging_state_notify(cur_stage_level, SOLO_CHALLENGE_NOTIFY_COMPLETE_FAILED);
		return;
	}

	DATA_TYPE dt;
	SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG* conf = (SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG*)world_manager::GetDataMan().get_data_ptr(SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if ((conf == NULL) || (dt != DT_SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG))
	{
		pImp->_runner->solo_challenge_challenging_state_notify(cur_stage_level, SOLO_CHALLENGE_NOTIFY_COMPLETE_FAILED);
		return;
	}
	
	SET_CV(CUR_STAGE_STATE,STAGE_COMPLETE);
	UpdateTimer(pImp);
	award_info.clear();
	int draw_award_times = 0;
	_left_draw_award_times = 0;
	
	if(IsTopLevel(cur_stage_level))//关卡>最大关卡，奖励道具
	{
		UpdateMaxStageLevel(cur_stage_level, pImp);

		draw_award_times = conf->level[cur_stage_level - 1].draw_award_times;//可抽宝箱的个数		
		_left_draw_award_times = draw_award_times;
		int award_page_config_id = conf->level[cur_stage_level -1].award_page_config_id;//奖励配置表id
		
		SOLO_TOWER_CHALLENGE_AWARD_PAGE_CONFIG* conf_award = (SOLO_TOWER_CHALLENGE_AWARD_PAGE_CONFIG*)world_manager::GetDataMan().get_data_ptr(award_page_config_id, ID_SPACE_CONFIG, dt);
		if ((conf_award == NULL) || (dt != DT_SOLO_TOWER_CHALLENGE_AWARD_PAGE_CONFIG))
		{
			pImp->_runner->solo_challenge_challenging_state_notify(cur_stage_level, SOLO_CHALLENGE_NOTIFY_COMPLETE_FAILED);
			return;
		}

		if(_max_stage_level == SOLO_TOWER_CHALLENGE_MAX_STAGE)
		{
			GMSV::SendUpdateSoloChallengeRank(pImp->GetParent()->ID.id, _total_first_climbing_time);
			GLog::formatlog("SOLO_CHALLENGE_LOG MAXSTAGE 用户 %d 闯过最高关卡,通关总用时 %d, 时间 %ld.", pImp->_parent->ID.id, _total_first_climbing_time, g_timer.get_systime());
		}
	}

	_total_score += conf->level[cur_stage_level - 1].award_score;//增长积分上限
	_cur_score   += conf->level[cur_stage_level - 1].award_score;//增长当前积分
	pImp->_runner->solo_challenge_challenging_state_notify(cur_stage_level, SOLO_CHALLENGE_NOTIFY_COMPLETE_SUCCESS);
	
	pImp->_runner->solo_challenge_award_info_notify(_max_stage_level, _total_first_climbing_time, _total_score, _cur_score, cur_stage_level, _cur_stage_cost_time, draw_award_times, award_info.size(), award_info);

	GLog::log(GLOG_INFO,"SOLO_CHALLENGE_LOG THROUGH_STAGE 用户 %d 闯过第 %d 关,时间 %ld,积分上限 %d ,当前积分 %d ",pImp->_parent->ID.id,cur_stage_level,g_timer.get_systime(),_total_score,_cur_score);
	_cur_stage_cost_time = 0;
}

void playersolochallenge::StageStart(gplayer_imp *pImp, bool isStartSuccess)
{
	int cur_stage_level = GET_CV(CUR_STAGE_LEVEL);

	int stage_state = GET_CV(CUR_STAGE_STATE);
	if(stage_state != STAGE_WAIT_START || !isStartSuccess)
	{
		pImp->_runner->solo_challenge_challenging_state_notify(cur_stage_level, SOLO_CHALLENGE_NOTIFY_START_FAILED);
		SET_CV(CUR_STAGE_LEVEL,-1);
		return;
	}

	SET_CV(CUR_STAGE_STATE,STAGE_PROCESSING);
	SET_CV(CUR_STAGE_START_TIMESTAMP,g_timer.get_systime());

	GLog::log(GLOG_INFO,"SOLO_CHALLENGE_LOG STAGE_START 用户%d 第 %d 关开始,时间 %ld, 玩法 %d, 玩法记录 %d.",pImp->_parent->ID.id, cur_stage_level, g_timer.get_systime(), _cur_stage_play_mode, _play_modes);
	pImp->_runner->solo_challenge_challenging_state_notify(cur_stage_level, SOLO_CHALLENGE_NOTIFY_START_SUCCESS);
}

void playersolochallenge::UpdateTimer(gplayer_imp *pImp) 
{
	int start_time = GET_CV(CUR_STAGE_START_TIMESTAMP);
	int cur_stage_level = GET_CV(CUR_STAGE_LEVEL);
	
 	_cur_stage_cost_time = g_timer.get_systime() - start_time;//此次闯关所用时间

	if(IsTopLevel(cur_stage_level))
	{
		_total_first_climbing_time += _cur_stage_cost_time;//更新闯关总时间
		_max_stage_cost_time = _cur_stage_cost_time;
	}
}

void playersolochallenge::UpdateMaxStageLevel(int stage_level, gplayer_imp *pImp)
{
	if(IsTopLevel(stage_level))
	{
		_max_stage_level = stage_level;
		SET_CV(MAX_STAGE_LEVEL,_max_stage_level);
		if(_cur_stage_play_mode != -1)
			_play_modes |= (1 << _cur_stage_play_mode);//记录新的玩法
		_cur_stage_play_mode = -1;
	}
}

int playersolochallenge::UserSelectAward(gplayer_imp *pImp, int stage_level, int args[])
{
 	if(_left_draw_award_times)//最大关卡，奖励道具
 	{
 		DATA_TYPE dt;
 		SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG* conf = (SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG*)world_manager::GetDataMan().get_data_ptr(SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG_ID, ID_SPACE_CONFIG, dt);
 		if ((conf == NULL) || (dt != DT_SOLO_TOWER_CHALLENGE_AWARD_LIST_CONFIG))
 		{
			return S2C::ERR_SOLO_CHALLENGE_AWARD_FAILURE;
 		}
 		
 		int award_page_config_id = conf->level[stage_level - 1].award_page_config_id;//奖励配置表id
 
 		//int draw_award_times = conf->level[stage_level - 1].draw_award_times;//可抽宝箱的个数
 		
 		SOLO_TOWER_CHALLENGE_AWARD_PAGE_CONFIG* conf_award = (SOLO_TOWER_CHALLENGE_AWARD_PAGE_CONFIG*)world_manager::GetDataMan().get_data_ptr(award_page_config_id, ID_SPACE_CONFIG, dt);
 		if ((conf_award == NULL) || (dt != DT_SOLO_TOWER_CHALLENGE_AWARD_PAGE_CONFIG))
 		{
			return S2C::ERR_SOLO_CHALLENGE_AWARD_FAILURE;
 		}
 		
		int index = abase::RandSelect(&(conf_award->list[0].probability),sizeof(conf_award->list[0]),20);//随机宝箱类型
		int item_id_rand = conf_award->list[index].id;
		
		int item_count_config = conf_award->list[index].count;

		//int item_count_rand = item_count_config;//abase::Rand(1,item_count_config);//进行宝箱个数的随机

		const item_data * pItem = (const item_data*)world_manager::GetDataMan().get_item_for_sell(item_id_rand);
		if(!pItem) 
		  return S2C::ERR_SOLO_CHALLENGE_AWARD_FAILURE;

		item_list & inventory = pImp->GetInventory();
		int pile_limit = pItem->pile_limit;
		size_t need_slot_count = item_count_config / pile_limit + (item_count_config % pile_limit?1:0);
		if(inventory.GetEmptySlotCount() < need_slot_count)
		  return S2C::ERR_INVENTORY_IS_FULL;
		item_data * pItem2 = DupeItem(*pItem);
		
		int guid1 = 0;
		int guid2 = 0;   
		if(pItem2->guid.guid1 != 0) 
		{ 
			 get_item_guid(item_id_rand, guid1,guid2); 
			 pItem2->guid.guid1 = guid1;
			 pItem2->guid.guid2 = guid2; 
		} 

		unsigned int totalcount = item_count_config;
		
		int effective_count = 0;
		while(totalcount)
		{
			int ocount = 0;
			if(totalcount > (unsigned int)pile_limit)
			{
				ocount = pile_limit;
				totalcount -= pile_limit;
			}
			else
			{
				ocount = totalcount;
				totalcount = 0;
			}

			pItem2->count = ocount;
			int rst = inventory.Push(*pItem2);
			ASSERT(rst >= 0 && pItem2->count == 0);
			if(rst >= 0)
			{
				effective_count += (ocount - pItem2->count);
				inventory[rst].InitFromShop();

				pImp->_runner->obtain_item(pItem2->type,pItem2->expire_date,ocount - pItem2->count,inventory[rst].count, 0,rst);
				if(pItem2->proc_type & item::ITEM_PROC_TYPE_AUTO_USE)
				{
					 pImp->UseItem(pImp->GetInventory(), rst, gplayer_imp::IL_INVENTORY, pItem2->type, 1);
				}
			}
			else
			  break;
		}
		FreeItem(pItem2);
		award_info.push_back(player_solo_challenge_award(item_id_rand, item_count_config));
		args[0] = item_id_rand;
		args[1] = effective_count;
		args[2] = award_info.size();
		--_left_draw_award_times;
		GLog::formatlog("SOLO_CHALLENGE_LOG AWARD 用户 %d 在单人副本获得 %d 个 %d ,剩余 %d 抽奖次数, 最大关卡 %d .",pImp->_parent->ID.id, effective_count,item_id_rand,_left_draw_award_times, _max_stage_level);
	}
	else
	{
		return S2C::ERR_SOLO_CHALLENGE_AWARD_FAILURE;
	}
	return 0;
}

int playersolochallenge::ScoreCost(gplayer_imp *pImp, int filter_index, int args[])
{
	if(!world_manager::GetIsSoloTowerChallengeInstance())
	{
		return S2C::ERR_SOLO_CHALLENGE_SCORE_COST;
	}

	DATA_TYPE dt;
	SOLO_TOWER_CHALLENGE_SCORE_COST_CONFIG* conf_score_cost = (SOLO_TOWER_CHALLENGE_SCORE_COST_CONFIG*)world_manager::GetDataMan().get_data_ptr(SOLO_TOWER_CHALLENGE_SCORE_COST_CONFIG_ID, ID_SPACE_CONFIG, dt);
	if ((conf_score_cost == NULL) || (dt != DT_SOLO_TOWER_CHALLENGE_SCORE_COST_CONFIG))
	{
		return S2C::ERR_SOLO_CHALLENGE_SCORE_COST;
	}
	if((unsigned int)_cur_score < conf_score_cost->score_buff_list[filter_index].score_cost)
	{
		return S2C::ERR_SOLO_CHALLENGE_SCORE_TOO_FEW;
	}
	if(conf_score_cost->score_buff_list[filter_index].cooldown_id != 0)
	{
		if(conf_score_cost->score_buff_list[filter_index].cooldown_id < COOLDOWN_INDEX_SOLO_CHALLENGE_BUFF_START || conf_score_cost->score_buff_list[filter_index].cooldown_id >COOLDOWN_INDEX_SOLO_CHALLENGE_BUFF_END)
		{
			return S2C::ERR_SOLO_CHALLENGE_SCORE_COST_COOLDOWN;
		}
		if(!pImp->CheckCoolDown(conf_score_cost->score_buff_list[filter_index].cooldown_id))
		{
			return S2C::ERR_SOLO_CHALLENGE_SCORE_COST_COOLDOWN;
		}
		pImp->SetCoolDown(conf_score_cost->score_buff_list[filter_index].cooldown_id, conf_score_cost->score_buff_list[filter_index].cooldown_time);
	}

	if(_filter_num[filter_index] >= filter_max_num[filter_index])
		return S2C::ERR_SOLO_CHALLENGE_FILTER_STACK_MAX;
	
	pImp->_skill.SoloChallengeAddFilter(object_interface(pImp), filter_index + FILTER_SOLO_START, conf_score_cost->score_buff_list[filter_index].param);	
	_cur_score -= conf_score_cost->score_buff_list[filter_index].score_cost;
	args[0] = _cur_score;
	args[1] = filter_index;
	GLog::log(GLOG_INFO,"SOLO_CHALLENGE_LOG SCORE 用户%d 使用 %d 积分兑换技能 %d 剩余积分 %d,时间%ld",pImp->_parent->ID.id,conf_score_cost->score_buff_list[filter_index].score_cost,filter_index, _cur_score, g_timer.get_systime());
	return 0;
}

int playersolochallenge::ClearFilter(gplayer_imp *pImp, int args[])
{
	for(int i = 0; i < ELEMENTDATA_NUM_SOLO_TOWER_CHALLENGE_BUFF_COUNT; i++)
	{
		pImp->_filters.RemoveFilter(FILTER_SOLO_START + i);//OnRelease()时会调用SetFilterData函数
	}
	_cur_score = _total_score;
	args[0]    = _cur_score;
	return 0;
}

void playersolochallenge::SetFilterData(int filter_id, int num, gplayer_imp *pImp)
{
	int filter_index[1] = { filter_id - FILTER_SOLO_START };
	int filter_num[1]   = { num };
	pImp->_runner->solo_challenge_buff_info_notify(filter_index, filter_num, 1,_cur_score);
	_filter_num[filter_id - FILTER_SOLO_START] = num;
}

void playersolochallenge::PlayerDeliverSoloChallengeScore(gplayer_imp *pImp, int score)
{
	_total_score += score;
	_cur_score    = _total_score;
	pImp->_runner->solo_challenge_operate_result(C2S::SOLO_CHALLENGE_OPT_DELIVERSCORE, 0, _cur_score, _total_score, 0);
}

int playersolochallenge::PlayerSoloChallengeLeaveTheRoom(gplayer_imp *pImp)
{
	if(world_manager::GetIsSoloTowerChallengeInstance()) 
	{
		A3DVECTOR pos(-381.653f,34.0f,432.892f);
		pImp->LongJump(pos);
	}
	return 0;
}

void playersolochallenge::Load(archive & ar)
{
	int num;
	ar >> _max_stage_level >> _max_stage_cost_time >> _total_score >> _total_first_climbing_time >> _left_draw_award_times >> _play_modes >> _cur_stage_cost_time >> _cur_stage_play_mode;
	ar >> num;
	player_solo_challenge_award tmp;
	for(int i = 0; i < num ; i++)
	{
		ar >> tmp.item_id;
		ar >> tmp.item_count;
		award_info.push_back(tmp);
	}
}

void playersolochallenge::Save(archive & ar)
{
	ar << _max_stage_level << _max_stage_cost_time << _total_score << _total_first_climbing_time << _left_draw_award_times << _play_modes << _cur_stage_cost_time << _cur_stage_play_mode;
	ar << award_info.size();
	for(size_t i = 0; i < award_info.size(); i++)
	{
		ar << award_info[i].item_id;
		ar << award_info[i].item_count;
	}
}

void playersolochallenge::Swap(playersolochallenge & rhs)
{
	abase::swap(_max_stage_level, rhs._max_stage_level);
	abase::swap(_max_stage_cost_time, rhs._max_stage_cost_time);
	abase::swap(_total_score, rhs._total_score);
	abase::swap(_total_first_climbing_time, rhs._total_first_climbing_time);
	abase::swap(_left_draw_award_times, rhs._left_draw_award_times);
	abase::swap(_play_modes, rhs._play_modes);
	abase::swap(_cur_stage_cost_time, rhs._cur_stage_cost_time);
	abase::swap(_cur_stage_play_mode, rhs._cur_stage_play_mode);
	award_info.swap(rhs.award_info);
}

void playersolochallenge::SetDBSoloChallengeInfo(const GDB::base_info::solo_challenge_info_t& solo_challenge_info)
{
	_max_stage_level            = solo_challenge_info.max_stage_level;
	_max_stage_cost_time        = solo_challenge_info.max_stage_cost_time;
	_total_score                = solo_challenge_info.total_score;
	_total_first_climbing_time  = solo_challenge_info.total_time;
	_left_draw_award_times      = solo_challenge_info.left_draw_award_times;
	_play_modes                 = solo_challenge_info.playmodes;
	award_info.clear();
	for(int i = 0; i < 8; i++)
	{
		if(solo_challenge_info.award_info[i].item_id != 0)
		{
			player_solo_challenge_award tmp;
			tmp.item_id    = solo_challenge_info.award_info[i].item_id;
			tmp.item_count = solo_challenge_info.award_info[i].item_count;
			award_info.push_back(tmp);
		}
		else
		  break;
	}
	_cur_score    = _total_score;
}

void playersolochallenge::GetDBSoloChallengeInfo(GDB::base_info::solo_challenge_info_t& solo_challenge_info)
{
	memset(&solo_challenge_info, sizeof(solo_challenge_info), 0);
	solo_challenge_info.max_stage_level              = _max_stage_level;
	solo_challenge_info.max_stage_cost_time          = _max_stage_cost_time;
	solo_challenge_info.total_score                  = _total_score;
	solo_challenge_info.total_time                   = _total_first_climbing_time;
	solo_challenge_info.left_draw_award_times        = _left_draw_award_times;
	solo_challenge_info.playmodes                    = _play_modes;
	size_t i=0;
	for(; 
		i < award_info.size() && i < sizeof(solo_challenge_info.award_info)/sizeof(solo_challenge_info.award_info[0]); 
		i++)
	{
		solo_challenge_info.award_info[i].item_id = award_info[i].item_id;
		solo_challenge_info.award_info[i].item_count = award_info[i].item_count;
	}
}

void playersolochallenge::ReSetSoloChallengeData()
{
	_max_stage_level             = 0;
	_max_stage_cost_time         = 0;
	_total_score                 = 0;
	_total_first_climbing_time   = 0;
	_left_draw_award_times       = 0;
	_play_modes                  = 0;
	_cur_stage_cost_time         = 0;
	_cur_score                   = 0;
	_cur_stage_play_mode         = -1;
	award_info.clear();
}

void playersolochallenge::RecordReSetLog(gplayer_imp *pImp)
{
	GLog::formatlog("SOLO_CHALLENGE_LOG RESET 用户 %d 本周达到的最高关卡数为: %d , 消耗的总时间 %d , 积分上限为 %d .",pImp->_parent->ID.id, _max_stage_level, _total_first_climbing_time, _total_score);
}

void playersolochallenge::OnClock(gplayer_imp *pImp)
{
	struct tm tt;
	int nowtime = g_timer.get_systime(); 
	localtime_r((time_t *)&nowtime, &tt);
	if(tt.tm_wday != 3)
		return;

	RecordReSetLog(pImp);
	ReSetSoloChallengeData();
	pImp->_runner->solo_challenge_award_info_notify(_max_stage_level, _total_first_climbing_time, _total_score, _cur_score, 0, 0, 0, award_info.size(), award_info);
	GLog::formatlog("SOLO_CHALLENGE_LOG RESET KICKOUT 用户 %d 单人副本数据重置.",pImp->_parent->ID.id);

	if(world_manager::GetIsSoloTowerChallengeInstance()) 
	{
		world_pos kickout_pos = world_manager::GetKickoutPoint();
		pImp->LongJump(kickout_pos.pos, kickout_pos.tag);
	}
}

void playersolochallenge::OnPassClock(gplayer_imp *pImp, int lastupdate,int now)
{
	struct tm tt; 
	localtime_r((time_t *)&now, &tt);
	int diff_day = 0;
	if(tt.tm_wday > 3)
	{
		diff_day = tt.tm_wday - 3;
	}
	else if(tt.tm_wday == 3)
	{
		if(tt.tm_hour < 7)
		  diff_day = tt.tm_wday + 7 - 3;
	}
	else
	{
		diff_day = tt.tm_wday + 7 - 3;
	}
	int diff_second = diff_day * 3600 * 24;
	tt.tm_sec  = 0;
	tt.tm_min  = 0;
	tt.tm_hour = 7;
	int today7hour = mktime(&tt);
	int lastestreset = today7hour - diff_second;
	if(lastupdate <= lastestreset)
	{
		RecordReSetLog(pImp);
		ReSetSoloChallengeData();
		pImp->_runner->solo_challenge_award_info_notify(_max_stage_level, _total_first_climbing_time, _total_score, _cur_score, 0, 0, 0, award_info.size(), award_info);
		GLog::formatlog("SOLO_CHALLENGE_LOG RESET 用户%d 单人副本数据重置.",pImp->_parent->ID.id);
	}
}
