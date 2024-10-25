#include "autoteamman.h"
#include "mapuser.h"
#include "autoteamsetgoal_re.hpp"
#include "autoteamplayerready.hpp"
#include "autoteamcomposestart.hpp"
#include "autoteamcomposefailed.hpp"
#include "autoteamplayerleave.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

AutoTeamMan AutoTeamMan::_instance; 

bool AutoTeamMan::Initialize()
{
	IntervalTimer::Attach(this, 1000000/IntervalTimer::Resolution());
	LOG_TRACE( "AutoTeamMan Init Successfully!!\n");
	return true;
}

void AutoTeamMan::PlayerChangeState(PlayerGoalEntry& entry, int roleid, char new_status, int reason)
{
	//LOG_TRACE("AutoTeam Player Change State: roleid=%d, old_status=%d, new_status=%d, reason=%d", roleid, entry._status, new_status, reason);
	entry._status = new_status;
}

void AutoTeamMan::UpdateTimeoutCandicates(std::vector<CandicateEntry>& candicates)
{
	int check_cnt = 0;
	time_t now = Timer::GetTime();
	
	std::vector<CandicateEntry>::iterator it = candicates.begin();
	while(it != candicates.end()) {
		if(++check_cnt > TIMEOUT_CHECK_CNT) return;
		
		int roleid = it->_roleid;
		if((now - it->_time) >= WAIT_TIMEOUT) {
			it = candicates.erase(it);	
			_players.erase(roleid);
			NotifyPlayerLeave(roleid, REASON_TIMEOUT);
			LOG_TRACE("AutoTeam Player Timeout : roleid=%d", roleid);
			continue;
		} else { //因为candicates的组织是按照FIFO的原则，前面的肯定比后面的时间早，故这个role不超时，后面的也必然不超时
			return;
		}
	}
}

void AutoTeamMan::UpdateTimeoutActivityGoalCandicates(ActivityGoal& goal)
{
	CANDICATE_MAP& can_map = goal._candicates;
	for(CANDICATE_MAP::iterator it = can_map.begin(); it != can_map.end(); ++it) {
		std::vector<CandicateEntry>& vec = it->second;
		UpdateTimeoutCandicates(vec);
	}
}

void AutoTeamMan::DoComposePreparedPlayers(std::vector<int>& prepared_players, std::vector<CandicateEntry>& candicates, int cnt)
{
	if((int)candicates.size() < cnt) {
		Log::log(LOG_ERR, "AutoTeam Player DoComposePreparedPlayers Error, candicate_cnt=%d, need_cnt=%d", candicates.size(), cnt);
		return;
	}

	for(int i = 0; i < cnt; ++i) {
		prepared_players.push_back(candicates[i]._roleid);
	}
	candicates.erase(candicates.begin(), candicates.begin() + cnt);
}

void AutoTeamMan::ComposeActivityPreparedPlayers(ActivityGoal& goal)
{
	CANDICATE_MAP::iterator it_free = goal._candicates.find(OCCUPATION_FREE_KEY);
	if(it_free == goal._candicates.end()) return;
	
	std::vector<CandicateEntry>& vec_free = it_free->second;
	if((int)vec_free.size() < (int)goal._occupation_free_need_player_cnt) return;

	OCCUPATION_LIMIT_MAP& limits = goal._occupation_limits;
	for(OCCUPATION_LIMIT_MAP::iterator it = limits.begin(); it != limits.end(); ++it) {
		char occupation = it->first;
		unsigned int occupation_need = it->second;

		CANDICATE_MAP::iterator it_can = goal._candicates.find(occupation);
		if(it_can == goal._candicates.end()) return;

		std::vector<CandicateEntry>& vec = it_can->second;
		if(vec.size() < occupation_need) return;
	}

	PreparedEntry entry;
	entry._timeout = PREPARED_TIMEOUT;

	for(OCCUPATION_LIMIT_MAP::iterator it = limits.begin(); it != limits.end(); ++it) {
		char occupation = it->first;
		unsigned int occupation_need = it->second;

		CANDICATE_MAP::iterator it_can = goal._candicates.find(occupation);
		std::vector<CandicateEntry>& vec = it_can->second;
		DoComposePreparedPlayers(entry._players, vec, occupation_need);
	}
	
	DoComposePreparedPlayers(entry._players, vec_free, goal._occupation_free_need_player_cnt);

	if(entry._players.empty()) return;
	int leader_idx = rand() % entry._players.size();
	entry._leader_id = entry._players[leader_idx];
	goal._prepared_players.push_back(entry);
	
	NotifyPlayersReady(goal._id, GOAL_TYPE_ACTIVITY, entry);
}

void AutoTeamMan::NotifyPlayersReady(int goal_id, char goal_type, const PreparedEntry& entry)
{
	AutoTeamPlayerReady	pro;
	pro.leader_id = entry._leader_id;
	
	for(unsigned int i = 0; i < entry._players.size(); ++i) {
		pro.roleid = entry._players[i];
		
		PLAYER_GOAL_MAP::iterator it_player = _players.find(pro.roleid);
		if(it_player == _players.end()) continue;
		if(it_player->second._goal_id != goal_id) continue;
		if(it_player->second._goal_type != goal_type) continue;
		
		PlayerChangeState(it_player->second, pro.roleid, STATUS_PLAYER_PREPARED, REASON_PLAYER_PREPARED);
		
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(pro.roleid);
		if(pInfo == NULL) continue;
		GProviderServer::GetInstance()->DispatchProtocol(pInfo->user->gameid, pro);
		//LOG_TRACE("AutoTeam Player Ready Send: goal_id=%d, goal_type=%d, roleid=%d, leader_id=%d, team_size=%d", goal_id, goal_type, pro.roleid, pro.leader_id, entry._players.size());
	}
}

void AutoTeamMan::NotifyPlayersComposeTeam(int goal_id, const PreparedEntry& entry, char goal_type)
{
	PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(entry._leader_id);
	if(pInfo == NULL) return;

	AutoTeamComposeStart pro;
	pro.goal_id = goal_id;
	pro.roleid = entry._leader_id;
	pro.member_list = entry._players;
	GProviderServer::GetInstance()->DispatchProtocol(pInfo->user->gameid, pro);
	//LOG_TRACE("AutoTeam Player Compose Team Notify: goal_id=%d, leader_id=%d, team_size=%d", goal_id, entry._leader_id, entry._players.size());

	for(unsigned int i = 0; i < entry._players.size(); ++i) {
		_players.erase(entry._players[i]);
		NotifyPlayerLeave(entry._players[i], REASON_COMPOSE_TEAM);
	}
}

void AutoTeamMan::NotifyPlayerLeave(int roleid, char reason)
{
	PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
	if(pInfo == NULL) return;

	AutoTeamPlayerLeave pro;
	pro.roleid = roleid;
	pro.reason = reason;
	pro.localsid = pInfo->localsid;

	GDeliveryServer::GetInstance()->Send(pInfo->linksid, pro);
}

void AutoTeamMan::NotifyPlayerComposeTeamFailed(int roleid, int leader_id, int gs_id)
{
	AutoTeamComposeFailed pro;
	pro.roleid = roleid;
	pro.leader_id = leader_id;
	GProviderServer::GetInstance()->DispatchProtocol(gs_id, pro);
}

void AutoTeamMan::ActivityPreparedPlayersError(const PreparedEntry& entry, CANDICATE_MAP& candicates)
{
	for(unsigned int i = 0; i < entry._players.size(); ++i) {
		int roleid =  entry._players[i];
		PLAYER_GOAL_MAP::iterator it_player = _players.find(roleid);
		if(it_player == _players.end()) continue;
		
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(pInfo == NULL) {
			_players.erase(roleid);
			continue;
		}

		if(it_player->second._status == STATUS_PLAYER_PREPARED || it_player->second._status == STATUS_PLAYER_READY) {
			PlayerChangeState(it_player->second, roleid, STATUS_PLAYER_NORMAL, REASON_PLAYER_PREPARED_ERR);

			char occupation = pInfo->cls;
			CANDICATE_MAP::iterator it_can = candicates.find(occupation);
			if(it_can == candicates.end()) {
				it_can = candicates.find(OCCUPATION_FREE_KEY);
			}

			CandicateEntry tmp_player;
			tmp_player._roleid = roleid;
			tmp_player._time = Timer::GetTime();

			std::vector<CandicateEntry>& vec = it_can->second;
			vec.push_back(tmp_player);
			
			NotifyPlayerComposeTeamFailed(roleid, entry._leader_id, pInfo->user->gameid);
		} else if(it_player->second._status == STATUS_PLAYER_INVALID) {
			_players.erase(roleid);
			NotifyPlayerLeave(roleid, REASON_INVALID_STATUS);
		}	
	}
}

char AutoTeamMan::GetPreparedGroupStatus(const std::vector<int>& players)
{
	char group_status = STATUS_PLAYER_READY;
	for(unsigned int i = 0; i < players.size(); ++i) {
		PLAYER_GOAL_MAP::iterator it_player = _players.find(players[i]);
		if(it_player == _players.end()) {
			group_status = STATUS_PLAYER_INVALID;
			break;
		}

		char status = it_player->second._status;
		if(status == STATUS_PLAYER_INVALID) {
			group_status = status;
			break;
		} else if(status == STATUS_PLAYER_PREPARED) {
			group_status = status;
		} else if(status == STATUS_PLAYER_READY){
			continue;
		} else {
			group_status = STATUS_PLAYER_INVALID;
			break;
		}	
	}
	
	return group_status;
}

void AutoTeamMan::UpdateActivityPreparedPlayers(ActivityGoal& goal)
{
	PREPARED_PLAYERS& list = goal._prepared_players;
	PREPARED_PLAYERS::iterator it = list.begin();
	while(it != list.end()) {
		PreparedEntry& entry = *it;
		--entry._timeout;
		
		char group_status = GetPreparedGroupStatus(entry._players);

		if(group_status == STATUS_PLAYER_READY)	{
			NotifyPlayersComposeTeam(goal._id, entry, GOAL_TYPE_ACTIVITY);
			it = list.erase(it);
			continue;
		} else if(group_status == STATUS_PLAYER_PREPARED) {
			if(entry._timeout == 0) {
				ActivityPreparedPlayersError(entry, goal._candicates);
				it = list.erase(it);
				continue;
			}
		} else { //INVALID
			ActivityPreparedPlayersError(entry, goal._candicates);
			it = list.erase(it);
			continue;
		}
		
		++it;
	}
}

void AutoTeamMan::ComposeTaskPreparedPlayers(TaskGoal& goal)
{
	TASK_CANDICATES& candicates = goal._task_candicates;
	if(candicates.size() < TASK_TEAM_MEMBER_MIN) return;

	PreparedEntry entry;
	entry._timeout = PREPARED_TIMEOUT;

	int member_cnt = candicates.size();
	if(member_cnt >= TASK_TEAM_MEMBER_MAX)  member_cnt = TASK_TEAM_MEMBER_MAX;

	DoComposePreparedPlayers(entry._players, candicates, member_cnt);

	if(entry._players.empty()) return;
	int leader_idx = rand() % entry._players.size();
	entry._leader_id = entry._players[leader_idx];
	goal._prepared_players.push_back(entry);
	
	NotifyPlayersReady(goal._id, GOAL_TYPE_TASK, entry);
}

void AutoTeamMan::UpdateTaskPreparedPlayers(TaskGoal& goal)
{
	PREPARED_PLAYERS& list = goal._prepared_players;
	PREPARED_PLAYERS::iterator it = list.begin();
	while(it != list.end()) {
		PreparedEntry& entry = *it;
		--entry._timeout;
		
		char group_status = GetPreparedGroupStatus(entry._players);

		if(group_status == STATUS_PLAYER_READY)	{
			NotifyPlayersComposeTeam(goal._id, entry, GOAL_TYPE_TASK);
			it = list.erase(it);
			continue;
		} else if(group_status == STATUS_PLAYER_PREPARED) {
			if(entry._timeout == 0) {
				TaskPreparedPlayersError(entry, goal._task_candicates);
				it = list.erase(it);
				continue;
			}
		} else { //INVALID
			TaskPreparedPlayersError(entry, goal._task_candicates);
			it = list.erase(it);
			continue;
		}
		
		++it;
	}
}

void AutoTeamMan::TaskPreparedPlayersError(const PreparedEntry& entry, TASK_CANDICATES& candicates)
{
	for(unsigned int i = 0; i < entry._players.size(); ++i) {
		int roleid =  entry._players[i];
		PLAYER_GOAL_MAP::iterator it_player = _players.find(roleid);
		if(it_player == _players.end()) continue;
		
		PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(pInfo == NULL) {
			_players.erase(roleid);
			continue;
		}

		if(it_player->second._status == STATUS_PLAYER_PREPARED || it_player->second._status == STATUS_PLAYER_READY) {
			PlayerChangeState(it_player->second, roleid, STATUS_PLAYER_NORMAL, REASON_PLAYER_PREPARED_ERR);
			
			CandicateEntry tmp_player;
			tmp_player._roleid = roleid;
			tmp_player._time = Timer::GetTime();

			candicates.push_back(tmp_player);
			NotifyPlayerComposeTeamFailed(roleid, entry._leader_id, pInfo->user->gameid);
		} else if(it_player->second._status == STATUS_PLAYER_INVALID){
			_players.erase(roleid);
			NotifyPlayerLeave(roleid, REASON_INVALID_STATUS);
		}	
	}

}

bool AutoTeamMan::Update()
{
	for(ACT_GOAL_MAP::iterator it = _act_goals.begin(); it != _act_goals.end(); ++it) {
		ActivityGoal& goal = it->second;	
		
		UpdateTimeoutActivityGoalCandicates(goal);
		ComposeActivityPreparedPlayers(goal);
		UpdateActivityPreparedPlayers(goal);
	}
	
	if(++_task_update_interval < TASK_UPDATE_INTERVAL) return true;

	TASK_GOAL_MAP::iterator it_task = _task_goals.begin();
	while(it_task != _task_goals.end()) {
		TaskGoal& goal = it_task->second;	
		if(goal._task_candicates.empty() && goal._prepared_players.empty()) {
			++goal._try_cnt;
			if(goal._try_cnt > TASK_TRY_CNT_MAX) {
				LOG_TRACE("AutoTeam Sys Remove Task Goal: goal_id=%d", goal._id);
				_task_goals.erase(it_task++);
				continue;
			}
		} else {
			goal._try_cnt = 0;
			UpdateTimeoutCandicates(goal._task_candicates);
			ComposeTaskPreparedPlayers(goal);
			UpdateTaskPreparedPlayers(goal);
		}

		++it_task;
	}
	
	_task_update_interval = 0;
	return true;
}

bool AutoTeamMan::RegisterGoalConfig(const std::vector<AutoTeamConfigData>& config_data)
{
	if(_status != STATUS_CONFIG_UNLOAD) return true;
	
	for(unsigned int i = 0; i < config_data.size(); ++i) {
		const AutoTeamConfigData& data = config_data[i];

		ActivityGoal goal;
		goal._id = data.goal_id;
		goal._need_player_cnt = data.need_player_cnt;
		
		std::vector<CandicateEntry> vec;
		char occupation_need_cnt = 0;
		for(unsigned int j = 0; j < data.occupation_info.size(); ++j) {
			const std::pair<char, char>& occupation = data.occupation_info[j];
			goal._occupation_limits[occupation.first] = occupation.second;
			goal._candicates[occupation.first] = vec;

			occupation_need_cnt += occupation.second;
		}
		
		goal._occupation_free_need_player_cnt = (goal._need_player_cnt - occupation_need_cnt);
		goal._candicates[OCCUPATION_FREE_KEY] = vec;

		_act_goals[goal._id] = goal;
	}
	
	_status = STATUS_CONFIG_LOADED;
	LOG_TRACE("AutoTeam Register Goal Config Complete: size=%d.\n", config_data.size());

	return true;
}

int AutoTeamMan::AddPlayerActivityGoal(int roleid, int goal_id, char occupation)
{
	ACT_GOAL_MAP::iterator it_goal = _act_goals.find(goal_id);
	if(it_goal == _act_goals.end()) return ERR_INVALID_GOAL;
	
	CANDICATE_MAP& candicates = it_goal->second._candicates;
	CANDICATE_MAP::iterator it_can = candicates.find(occupation);
	if(it_can == candicates.end()) {
		it_can = candicates.find(OCCUPATION_FREE_KEY);
	}

	CandicateEntry player;
	player._roleid = roleid;
	player._time = Timer::GetTime();
	
	std::vector<CandicateEntry>& vec = it_can->second;
	vec.push_back(player);
	
	PlayerGoalEntry entry = {goal_id, GOAL_TYPE_ACTIVITY, STATUS_PLAYER_NORMAL};
	_players[roleid] = entry;

	//LOG_TRACE("AutoTeam Player Add Activity Goal: roleid=%d, goal_id=%d, occupation=%d", roleid, goal_id, occupation);
	return ERR_SUCCESS;
}

int AutoTeamMan::RemovePlayerActivityGoal(int roleid, int goal_id, char occupation)
{
	PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
	if(it == _players.end()) return ERR_SUCCESS;
	if(it->second._goal_id != goal_id) return ERR_INVALID_GOAL;
	if(it->second._goal_type != GOAL_TYPE_ACTIVITY) return ERR_INVALID_GOAL;
	if(it->second._status != STATUS_PLAYER_NORMAL) return ERR_INVALID_ROLE_STATUS;

	ACT_GOAL_MAP::iterator it_goal = _act_goals.find(goal_id);
	if(it_goal == _act_goals.end()) return ERR_INVALID_GOAL;
	CANDICATE_MAP& candicates = it_goal->second._candicates;
	
	CANDICATE_MAP::iterator it_can = candicates.find(occupation);
	if(it_can == candicates.end()) {
		it_can = candicates.find(OCCUPATION_FREE_KEY);
	}

	std::vector<CandicateEntry>& vec = it_can->second;
	for(std::vector<CandicateEntry>::iterator it_vec = vec.begin(); it_vec != vec.end(); ++it_vec) {
		if(it_vec->_roleid == roleid) {
			it_vec = vec.erase(it_vec);
			break;
		}
	}
	_players.erase(roleid);
	
	//LOG_TRACE("AutoTeam Player Remove Actvity Goal: roleid=%d, goal_id=%d, occupation=%d", roleid, goal_id, occupation);
	return ERR_SUCCESS;
}

int AutoTeamMan::PlayerSetActivityGoal(int roleid, int goal_id, char occupation, char op)
{
	int ret = ERR_INVALID_GOAL;
	if(op > 0) { //加入自动组队的候选
		PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
		if(it != _players.end()) { //玩家已经在队列中了
			if(it->second._goal_type == GOAL_TYPE_TASK) { //玩家在任务自动组队中
				ret = RemovePlayerTaskGoal(roleid, it->second._goal_id); //将玩家从任务自动组队移除
				if(ret == ERR_SUCCESS) { //移除成功
					//将玩家加入到新的目标队列中
					ret = AddPlayerActivityGoal(roleid, goal_id, occupation);
				} else {
					Log::log(LOG_ERR, "AutoTeam Player add activity goal error, alreay in task goal, roleid=%d, goal_id=%d, op=%d, ret=%d", roleid, goal_id, op, ret);
				}
			} else if (it->second._goal_type == GOAL_TYPE_ACTIVITY) { //玩家在活动，副本的自动组队中
				int now_goal_id = it->second._goal_id;
				if(now_goal_id != goal_id) {
					ret = RemovePlayerActivityGoal(roleid, now_goal_id, occupation); //将玩家移除 
					if(ret == ERR_SUCCESS) { //移除成功
						//将玩家加入到新的目标队列中
						ret = AddPlayerActivityGoal(roleid, goal_id, occupation);
					} else {
						Log::log(LOG_ERR, "AutoTeam Player recover activity goal error, roleid=%d, goal_id=%d, op=%d, ret=%d", roleid, goal_id, op, ret);
					}
				} else {
					ret = ERR_SUCCESS;
				}
			}
		} else {
			ret = AddPlayerActivityGoal(roleid, goal_id, occupation);
		}	
	} else { //离开自动组队系统
		char reason = REASON_PLAYER_OP;
		ret = RemovePlayerActivityGoal(roleid, goal_id, occupation);	
		if(ret == ERR_SUCCESS) {
			NotifyPlayerLeave(roleid, reason);
		} else {	
			Log::log(LOG_ERR, "AutoTeam Player remove activity goal error, roleid=%d, goal_id=%d, op=%d, ret=%d", roleid, goal_id, op, ret);
		}
	}

	return ret;
}

int AutoTeamMan::AddPlayerTaskGoal(int roleid, int goal_id)
{
	CandicateEntry player;
	player._roleid = roleid;
	player._time = Timer::GetTime();

	TASK_GOAL_MAP::iterator it_goal = _task_goals.find(goal_id);
	if(it_goal != _task_goals.end()) {
		TASK_CANDICATES& candicates = it_goal->second._task_candicates;
		candicates.push_back(player);
	} else {
		TaskGoal task_goal;
		task_goal._id = goal_id;
		task_goal._try_cnt = 0;
		task_goal._task_candicates.push_back(player);

		_task_goals[goal_id] = task_goal;
	}
	
	PlayerGoalEntry entry = {goal_id, GOAL_TYPE_TASK, STATUS_PLAYER_NORMAL};
	_players[roleid] = entry;

	//LOG_TRACE("AutoTeam Player Add Task Goal: roleid=%d, goal_id=%d", roleid, goal_id);
	return ERR_SUCCESS;
}

int AutoTeamMan::RemovePlayerTaskGoal(int roleid, int goal_id)
{
	PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
	if(it == _players.end()) return ERR_SUCCESS;
	if(it->second._goal_id != goal_id) return ERR_INVALID_GOAL;
	if(it->second._goal_type != GOAL_TYPE_TASK) return ERR_INVALID_GOAL;
	if(it->second._status != STATUS_PLAYER_NORMAL) return ERR_INVALID_ROLE_STATUS;

	TASK_GOAL_MAP::iterator it_goal = _task_goals.find(goal_id);
	if(it_goal == _task_goals.end()) return ERR_INVALID_GOAL;
	TASK_CANDICATES& candicates = it_goal->second._task_candicates;
	
	for(TASK_CANDICATES::iterator it_can = candicates.begin(); it_can != candicates.end(); ++it_can) {
		if(it_can->_roleid == roleid) {
			it_can = candicates.erase(it_can);
			break;
		}
	}
	_players.erase(roleid);
	
	//LOG_TRACE("AutoTeam Player Remove Task Goal: roleid=%d, goal_id=%d", roleid, goal_id);
	return ERR_SUCCESS;
}

int AutoTeamMan::PlayerSetTaskGoal(int roleid, int goal_id, char op)
{
	int ret = ERR_ALREADY_ACTIVITY;
	if(op > 0) {
		PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
		if(it != _players.end()) { //玩家已经在队列中了
			PlayerGoalEntry& entry = it->second;
			if(entry._goal_type >= GOAL_TYPE_ACTIVITY) {
				Log::log(LOG_ERR, "AutoTeam Player add task goal error, roleid=%d, goal_id=%d, op=%d, ret=%d", roleid, goal_id, op, ret);
				return ret;
			}
				
			int now_goal_id = entry._goal_id;
			if(now_goal_id != goal_id) {
				//先将玩家从上次的队列中移除
				ret = RemovePlayerTaskGoal(roleid, now_goal_id);
				if(ret == ERR_SUCCESS) { //移除成功
					//将玩家加入到新的目标队列中
					ret = AddPlayerTaskGoal(roleid, goal_id);
				} else {
					//Log::log(LOG_ERR, "AutoTeam Player recover task goal error, roleid=%d, goal_id=%d, op=%d, ret=%d", roleid, goal_id, op, ret);
				}
			} else {
				ret = ERR_SUCCESS;
			}
		} else {
			ret = AddPlayerTaskGoal(roleid, goal_id);
		}
	} else {
		char reason = REASON_PLAYER_OP;
		ret = RemovePlayerTaskGoal(roleid, goal_id);
		if(ret == ERR_SUCCESS) {
			NotifyPlayerLeave(roleid, reason);
		} else {
			//Log::log(LOG_ERR, "AutoTeam Player remove task goal error, roleid=%d, goal_id=%d, op=%d, ret=%d", roleid, goal_id, op, ret);
		}
	}

	return ret;
}

void AutoTeamMan::OnPlayerSetGoal(int roleid, char goal_type, char op, int goal_id, unsigned int sid)
{
	//LOG_TRACE("AutoTeam Player Set Goal: roleid=%d, goal_id=%d, op=%d, goal_type=%d, sid=%d", roleid, goal_id, op, goal_type, sid);

	int ret = ERR_INVALID_GOAL;
	PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline( roleid );
	if(pInfo == NULL) return;
	char occupation = pInfo->cls;

	if(goal_type == GOAL_TYPE_TASK) {
		ret = PlayerSetTaskGoal(roleid, goal_id, op);
	} else if(goal_type == GOAL_TYPE_ACTIVITY) {
		ret = PlayerSetActivityGoal(roleid, goal_id, occupation, op);
	}
		
	AutoTeamSetGoal_Re re(roleid, goal_type, op, goal_id, ret, pInfo->localsid);
	GDeliveryServer::GetInstance()->Send(pInfo->linksid, re);
}

void AutoTeamMan::OnPlayerReady(int roleid, int leader_id, int retcode)
{
	//LOG_TRACE("AutoTeam Player Ready Re: roleid=%d, leader_id=%d, retcode=%d", roleid, leader_id, retcode);
	PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
	if(it == _players.end()) return;
	if(it->second._status != STATUS_PLAYER_PREPARED) return;
	
	if(retcode == ERR_SUCCESS) {
		PlayerChangeState(it->second, roleid, STATUS_PLAYER_READY, REASON_PLAYER_READY);
	} else {
		PlayerChangeState(it->second, roleid, STATUS_PLAYER_INVALID, REASON_PLAYER_UNREADY);
	}
}

void AutoTeamMan::OnPlayerLogout(int roleid, char occupation)
{
	PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
	if(it == _players.end()) return;
	
	PlayerGoalEntry& entry = it->second;	
	if(entry._status == STATUS_PLAYER_NORMAL) {
		if(entry._goal_type == GOAL_TYPE_TASK) {
			RemovePlayerTaskGoal(roleid, entry._goal_id);
		} else if(entry._goal_type == GOAL_TYPE_ACTIVITY) {
			RemovePlayerActivityGoal(roleid, entry._goal_id, occupation);
		}
	} else {
		PlayerChangeState(entry, roleid, STATUS_PLAYER_INVALID, REASON_PLAYER_LOGOUT);
	}
}

void AutoTeamMan::OnPlayerJoinTeam(int roleid)
{
	PlayerInfo* pInfo = UserContainer::GetInstance().FindRoleOnline( roleid );
	if(pInfo == NULL) return;

	PLAYER_GOAL_MAP::iterator it = _players.find(roleid);
	if(it == _players.end()) return;
	
	PlayerGoalEntry& entry = it->second;	
	if(entry._status == STATUS_PLAYER_NORMAL) {
		int ret = ERR_INVALID_GOAL;
		if(entry._goal_type == GOAL_TYPE_TASK) {
			ret = RemovePlayerTaskGoal(roleid, entry._goal_id);
		} else if(entry._goal_type == GOAL_TYPE_ACTIVITY) {
			ret = RemovePlayerActivityGoal(roleid, entry._goal_id, pInfo->cls);
		}
		
		if(ret == ERR_SUCCESS) NotifyPlayerLeave(roleid, REASON_JOIN_TEAM);
	} else {
		PlayerChangeState(it->second, roleid, STATUS_PLAYER_INVALID, REASON_PLAYER_JOIN_TEAM);
	}
}

}

