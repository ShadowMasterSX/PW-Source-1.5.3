#ifndef __GNET_AUTO_TEAM_MAN_H
#define __GNET_AUTO_TEAM_MAN_H
#include <vector>
#include <map>

#include "autoteamconfigdata"

namespace GNET
{

class AutoTeamMan: public IntervalTimer::Observer
{
public:
	enum {
		STATUS_CONFIG_UNLOAD = 0,
		STATUS_CONFIG_LOADED,
	};
	
	enum {
		STATUS_PLAYER_INVALID = -1,
		STATUS_PLAYER_NORMAL = 0,
		STATUS_PLAYER_PREPARED,
		STATUS_PLAYER_READY,
	};

	enum {
		ERR_INVALID_GOAL = -1,
		ERR_INVALID_ROLE_STATUS = -2,
		ERR_ALREADY_ACTIVITY = -3,
	};
	
	enum {
		OCCUPATION_FREE_KEY = -1,
		PREPARED_TIMEOUT = 10,
		WAIT_TIMEOUT = 600,
		TASK_TEAM_MEMBER_MIN = 3,
		TASK_TEAM_MEMBER_MAX = 6,
		TASK_UPDATE_INTERVAL = 20,
		TASK_TRY_CNT_MAX = 30,
		TIMEOUT_CHECK_CNT = 50,
	};

	enum {
		REASON_COMPOSE_TEAM = 0,
		REASON_INVALID_STATUS,
		REASON_PLAYER_OP,
		REASON_JOIN_TEAM,
		REASON_TIMEOUT,
	};
	
	enum {
		REASON_PLAYER_PREPARED = 0,
		REASON_PLAYER_PREPARED_ERR,
		REASON_PLAYER_READY,
		REASON_PLAYER_UNREADY,
		REASON_PLAYER_LOGOUT,
		REASON_PLAYER_JOIN_TEAM,
	};
	
	enum {
		GOAL_TYPE_INVALID =0,
		GOAL_TYPE_TASK,
		GOAL_TYPE_ACTIVITY,
	};

	struct CandicateEntry
	{
		int _roleid;
		int _time;
	};
	
	typedef std::map<char/*occupation*/, char/*need_cnt*/> OCCUPATION_LIMIT_MAP;
	typedef std::map< char/*occupation*/, std::vector<CandicateEntry> > CANDICATE_MAP;
	
	struct PreparedEntry
	{
		int _leader_id;
		char _timeout;
		std::vector<int> _players;
	};
	
	typedef std::vector<PreparedEntry> PREPARED_PLAYERS;
	
	struct ActivityGoal
	{
		int _id;
		char _need_player_cnt;
		char _occupation_free_need_player_cnt;
		OCCUPATION_LIMIT_MAP _occupation_limits;
		CANDICATE_MAP _candicates;
		PREPARED_PLAYERS _prepared_players;
	};
	
	typedef std::vector<CandicateEntry> TASK_CANDICATES;
	struct TaskGoal
	{
		int _id;
		short _try_cnt;
		TASK_CANDICATES _task_candicates;
		PREPARED_PLAYERS _prepared_players;
	};
	
	struct PlayerGoalEntry
	{
		int _goal_id;
		char _goal_type;
		char _status;
	};
	
	typedef std::map<int/*goal_id*/, ActivityGoal> ACT_GOAL_MAP;
	typedef std::map<int/*task_id*/, TaskGoal> TASK_GOAL_MAP;
	//typedef std::map<int/*roleid*/, PlayerGoalEntry> PLAYER_GOAL_MAP;
	typedef __gnu_cxx::hash_map<int/*roleid*/, PlayerGoalEntry> PLAYER_GOAL_MAP;

private:
	int _status;
	int _task_update_interval;
	ACT_GOAL_MAP _act_goals;
	TASK_GOAL_MAP _task_goals;
	PLAYER_GOAL_MAP _players;
	static AutoTeamMan _instance; 

private:
	AutoTeamMan(): _status(STATUS_CONFIG_UNLOAD), _task_update_interval(0) { }
	
	int AddPlayerTaskGoal(int roleid, int goal_id);
	int RemovePlayerTaskGoal(int roleid, int goal_id);
	int PlayerSetTaskGoal(int roleid, int goal_id, char op);
	void ComposeTaskPreparedPlayers(TaskGoal& goal);
	void UpdateTaskPreparedPlayers(TaskGoal& goal);	
	void TaskPreparedPlayersError(const PreparedEntry& entry, TASK_CANDICATES& candicates);

	int AddPlayerActivityGoal(int roleid, int goal_id, char occupation);  
	int RemovePlayerActivityGoal(int roleid, int goal_id, char occupation);	
	int PlayerSetActivityGoal(int roleid, int goal_id, char occupation, char op);
	void ComposeActivityPreparedPlayers(ActivityGoal& goal);
	void UpdateActivityPreparedPlayers(ActivityGoal& goal);	
	void ActivityPreparedPlayersError(const PreparedEntry& entry, CANDICATE_MAP& candicates);
	void UpdateTimeoutActivityGoalCandicates(ActivityGoal& goal);

	void NotifyPlayersReady(int goal_id, char goal_type, const PreparedEntry& entry);
	void NotifyPlayersComposeTeam(int goal_id, const PreparedEntry& entry, char goal_type);
	void NotifyPlayerComposeTeamFailed(int roleid, int leader_id, int gs_id);
	void NotifyPlayerLeave(int roleid, char reason);
	void PlayerChangeState(PlayerGoalEntry& entry, int roleid, char new_status, int reason);
	char GetPreparedGroupStatus(const std::vector<int>& players);
	void UpdateTimeoutCandicates(std::vector<CandicateEntry>& candicates);
	void DoComposePreparedPlayers(std::vector<int>& prepared_players, std::vector<CandicateEntry>& candicates, int cnt);

public:
	static AutoTeamMan* GetInstance() { return &_instance; }
	bool Initialize();

	bool Update();
	bool RegisterGoalConfig(const std::vector<AutoTeamConfigData>& config_data);
	void OnPlayerSetGoal(int roleid, char goal_type, char op, int goal_id, unsigned int sid);
	void OnPlayerReady(int roleid, int leader_id, int retcode);
	void OnPlayerLogout(int roleid, char occupation);
	void OnPlayerJoinTeam(int roleid);
};

}

#endif //__GNET_AUTO_TEAM_MAN_H
