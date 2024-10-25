#ifndef __ONLINEGAME_GS_PUBLICQUEST_H__
#define __ONLINEGAME_GS_PUBLICQUEST_H__

#include <amemory.h>
#include <vector.h>
#include <hashmap.h>
#include <map>

//公共任务,由于任务进度使用了全局变量，所以只能在BIG_WORLD中使用
#define 	PQ_COMMONVALUE_NOTIFY_INTERVAL	5	//公共任务相关的全局变量5秒钟广播一次
#define		PQ_RANKS_SORT_INTERVAL 		60	//排行榜60秒排序一次	
#define		PQ_RANKS_NOTIFY_CLIENT		20	//将排行榜的前20名通知给客户端

class public_quest
{
public:
	enum
	{
		PQ_STATE_INVALID = -1,
		PQ_STATE_INITIALED,		//pq任务初始化完毕
		PQ_STATE_RUNNING,		//pq任务正在进行
		PQ_STATE_FINISHED,		//pq任务已经完成等待玩家领取奖励
	};

	struct player_pq_info
	{
		int roleid;
		int link_id;
		int link_sid;
		bool is_active;
		int race;	//职业加性别
		int level;
		int score;
		int cls_place;
		int all_place;
		int rand;	//-1代表未随机
	};

	typedef abase::hash_map<int/*roleid*/,struct player_pq_info*,abase::_hash_function,abase::fast_alloc<> > SCORE_MAP;
	typedef std::multimap<int/*score*/,struct player_pq_info*> CLS_RANKS;
	typedef abase::vector<CLS_RANKS,abase::fast_alloc<> > CLS_RANKS_LIST;

public:
	public_quest():_lock(0),_pq_state(PQ_STATE_INVALID),_heartbeat_counter(0),
					_task_id(0),_first_child_task_id(0),_cur_child_task_id(0),_cur_task_stamp(0),
					_score_changed(false),_no_change_rank(false),_ranks(USER_CLASS_COUNT+1,CLS_RANKS())
	{
	}
	~public_quest();
	void Init(int task_id, int child_task_id, int* common_value, int size);

	void Heartbeat(int tick);	//tick秒
	
	bool SetStart(int* common_value, int size, bool no_change_rank);
	bool SetFinish();
	bool SetNextChildTask(int child_task_id, int* common_value, int size, bool no_change_rank);
	bool SetRandContrib(int fixed_contrib, int max_rand_contrib, int lowest_contrib);
	int GetState();

	int GetSubTask();
	int GetTaskStamp();
	int GetContrib(int role_id);

	int GetAllPlace(int role_id);
	int GetClsPlace(int role_id);
	
	bool AddPlayer(int roleid);
	bool RemovePlayer(int roleid);
	bool UpdatePlayerContrib(int roleid, int inc_contrib);
	bool ClearPlayerContrib();

	void EnterWorldInit(int roleid);
	void LeaveWorld(int role_id);
private:
	void RestartClear();	//用于任务重置后清除数据
	
	void SortRanks();
	void BroadcastRanks();
	void BroadcastInfo();
	void BroadcastCommonvalue();
	
	void SendRanks(int link_id, int roleid, int link_sid);
	void SendInfo(int link_id, int roleid, int link_sid, int score, int cls_place, int all_place);
	void SendCommonvalue(int link_id, int roleid, int link_sid);
	void SendEmptyInfo(int link_id, int roleid, int link_sid);
protected:
	int _lock;
	
	int _pq_state;
	int _heartbeat_counter;

	int _task_id;
	int _first_child_task_id;
	int _cur_child_task_id;
	int _cur_task_stamp;
	std::map<int,int> _common_value;	//任务关联的全局变量索引表

	bool _score_changed;
	bool _no_change_rank;
	SCORE_MAP _score;
	CLS_RANKS_LIST _ranks;	//_ranks[0, USER_CLASS_COUNT-1]是各职业排行榜，_ranks[USER_CLASS_COUNT]是总排行榜
};

class public_quest_manager
{
public:
	typedef abase::hash_map<int/*task_id*/,public_quest *,abase::_hash_function,abase::fast_alloc<> > PUBLIC_QUEST_MAP;

public:
	public_quest_manager():_lock_map(0){}
	~public_quest_manager();

	void Heartbeat(int tick);	//tick秒

	bool InitAddQuest(int task_id, int child_task_id, int* common_value, int size);
	
	bool QuestSetStart(int task_id, int* common_value, int size, bool no_change_rank);
	bool QuestSetFinish(int task_id);
	bool QuestSetNextChildTask(int task_id, int child_task_id, int* common_value, int size, bool no_change_rank);
	bool QuestSetRandContrib(int task_id, int fixed_contrib, int max_rand_contrib, int lowest_contrib);
	int GetQuestState(int task_id);
	
	int GetCurSubTask(int task_id);
	int GetCurTaskStamp(int task_id);
	int GetCurContrib(int task_id, int role_id);

	int GetCurAllPlace(int task_id, int role_id);
	int GetCurClsPlace(int task_id, int role_id);
	
	bool QuestAddPlayer(int task_id, int role_id);
	bool QuestRemovePlayer(int task_id, int role_id);
	bool QuestUpdatePlayerContrib(int task_id, int roleid, int inc_contrib);
	
	void QuestEnterWorldInit(int task_id, int role_id);
	void QuestLeaveWorld(int task_id, int role_id);
private:
	public_quest * GetPublicQuest(int task_id);	
	
protected:
	int _lock_map;
	PUBLIC_QUEST_MAP _pq_map;

};

#endif
