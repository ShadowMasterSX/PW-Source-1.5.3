#ifndef __ONLINEGAME_GS_SOLO_CHALLENGE_H__
#define __ONLINEGAME_GS_SOLO_CHALLENGE_H__

#include <vector.h>
#include <common/base_wrapper.h>
#include "timer.h"
#include <db_if.h>
#include "template/exptypes.h"
#include <cstring>

#define GET_CV(type) pImp->_plane->GetCommonValue(COMMON_VALUE_ID_SOLO_CHALLENGE_##type)
#define SET_CV(type,value) pImp->_plane->SetCommonValue(COMMON_VALUE_ID_SOLO_CHALLENGE_##type,value)
#define FILTER_SOLO_START FILTER_SOLO_INCATTACKANDMAGIC

namespace
{
	const static int filter_max_num[16] = {20,20,20,20,
											1, 1, 1, 1,
										   20,20, 0, 0,
											0, 0, 0, 0
	};
}

class gplayer_imp;
class playersolochallenge
{
public:
	enum
	{
		STAGE_STATE_INIT     =0,
		STAGE_WAIT_START,
		STAGE_PROCESSING,
		STAGE_COMPLETE,

		STAGE_STATE_MAX,
	};

	enum 
	{
		CUR_STAGE_START_TIMESTAMP,
		CUR_STAGE_LEVEL,
		CUR_STAGE_STATE,
		MAX_STAGE_LEVEL,
		CUR_STAGE_PLAY_MODE,
	};
	
	enum
	{
		SOLO_CHALLENGE_NOTIFY_START_SUCCESS = 0,
		SOLO_CHALLENGE_NOTIFY_START_FAILED,
		SOLO_CHALLENGE_NOTIFY_COMPLETE_SUCCESS,
		SOLO_CHALLENGE_NOTIFY_COMPLETE_FAILED,
	};

	struct player_solo_challenge_award
	{
		int item_id;
		int item_count;
		player_solo_challenge_award():item_id(-1),item_count(-1)
		{
		}
		player_solo_challenge_award(int tmp_id, int tmp_count):item_id(tmp_id),item_count(tmp_count)
		{
		}
	};
private:
	int _max_stage_level;//已闯到的最高关卡
	int _max_stage_cost_time;//最高关卡的闯关时间
	int _total_score;//积分上限
	int _total_first_climbing_time;//闯关总时间，只记录第一次闯关时间的总和
	int _left_draw_award_times;
	int _play_modes;//已玩玩法
	abase::vector<player_solo_challenge_award> award_info;//存放最高关卡已抽到的奖励
	
	int _cur_stage_cost_time;//当前关卡的闯关时间
	int _cur_stage_play_mode;//当前关卡的玩法
	int _cur_score;
	int _filter_num[16];

private:
	bool IsTopLevel(int cur_stage_level)
	{
		return (cur_stage_level == (_max_stage_level + 1));
	}
	
public:
	playersolochallenge():_max_stage_level(0),_max_stage_cost_time(0),_total_score(0),_total_first_climbing_time(0),_left_draw_award_times(0),_play_modes(0),_cur_stage_cost_time(0),_cur_stage_play_mode(-1)
	{
		memset(&_filter_num, 0, sizeof(_filter_num));
	}
	~playersolochallenge(){}

	void Save(archive & ar);
	void Load(archive & ar);

	void Swap(playersolochallenge & rhs);

	void GetDBSoloChallengeInfo(GDB::base_info::               solo_challenge_info_t& solo_challenge_info);
	void SetDBSoloChallengeInfo(const GDB::base_info::               solo_challenge_info_t& solo_challenge_info);
	
	void SelectStage(gplayer_imp *pImp,int stage_level);
	void StageComplete(gplayer_imp *pImp, bool isCompleteSuccess);//任务完成后，进行奖励的随机
	void StageStart(gplayer_imp *pImp, bool isStartSuccess);//任务开始计时

	int UserSelectAward(gplayer_imp *pImp, int num, int args[]);//用户选择奖励
	int ScoreCost(gplayer_imp *pImp, int filter_index, int args[]);

	int ClearFilter(gplayer_imp *pImp, int args[]);
	void SetFilterData(int filter_id, int num, gplayer_imp *pImp);

	void NotifySoloChallengeData(gplayer_imp *pImp);//通知客户端单人副本数据
	void PlayerEnterSoloChallengeInstance(gplayer_imp *pImp);//玩家进入单人副本
	void PlayerLeaveSoloChallengeInstance(gplayer_imp *pImp);//玩家进入离开单人副本
	void PlayerDeliverSoloChallengeScore(gplayer_imp *pImp, int score);
	int PlayerSoloChallengeLeaveTheRoom(gplayer_imp *pImp);

	void OnClock(gplayer_imp *pImp);
	void OnPassClock(gplayer_imp *pImp, int lastupdate,int now);

	void SetMaxStageLevel(int max_stage_level, gplayer_imp *pImp)
	{
		_max_stage_level = max_stage_level;
		StageComplete(pImp, true);
	}
private:
	void UpdateTimer(gplayer_imp *pImp);//任务结束，记录闯关时间
	void UpdateMaxStageLevel(int stage_level, gplayer_imp *pImp);
	void CalcPlayMode(SOLO_TOWER_CHALLENGE_LEVEL_CONFIG *conf, int stage_level, gplayer_imp *pImp);

	void ReSetSoloChallengeData();//每周五零时重置数据
	void RecordReSetLog(gplayer_imp *pImp);
};

#endif
