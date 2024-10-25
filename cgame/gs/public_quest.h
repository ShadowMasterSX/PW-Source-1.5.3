#ifndef __ONLINEGAME_GS_PUBLICQUEST_H__
#define __ONLINEGAME_GS_PUBLICQUEST_H__

#include <amemory.h>
#include <vector.h>
#include <hashmap.h>
#include <map>

//��������,�����������ʹ����ȫ�ֱ���������ֻ����BIG_WORLD��ʹ��
#define 	PQ_COMMONVALUE_NOTIFY_INTERVAL	5	//����������ص�ȫ�ֱ���5���ӹ㲥һ��
#define		PQ_RANKS_SORT_INTERVAL 		60	//���а�60������һ��	
#define		PQ_RANKS_NOTIFY_CLIENT		20	//�����а��ǰ20��֪ͨ���ͻ���

class public_quest
{
public:
	enum
	{
		PQ_STATE_INVALID = -1,
		PQ_STATE_INITIALED,		//pq�����ʼ�����
		PQ_STATE_RUNNING,		//pq�������ڽ���
		PQ_STATE_FINISHED,		//pq�����Ѿ���ɵȴ������ȡ����
	};

	struct player_pq_info
	{
		int roleid;
		int link_id;
		int link_sid;
		bool is_active;
		int race;	//ְҵ���Ա�
		int level;
		int score;
		int cls_place;
		int all_place;
		int rand;	//-1����δ���
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

	void Heartbeat(int tick);	//tick��
	
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
	void RestartClear();	//�����������ú��������
	
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
	std::map<int,int> _common_value;	//���������ȫ�ֱ���������

	bool _score_changed;
	bool _no_change_rank;
	SCORE_MAP _score;
	CLS_RANKS_LIST _ranks;	//_ranks[0, USER_CLASS_COUNT-1]�Ǹ�ְҵ���а�_ranks[USER_CLASS_COUNT]�������а�
};

class public_quest_manager
{
public:
	typedef abase::hash_map<int/*task_id*/,public_quest *,abase::_hash_function,abase::fast_alloc<> > PUBLIC_QUEST_MAP;

public:
	public_quest_manager():_lock_map(0){}
	~public_quest_manager();

	void Heartbeat(int tick);	//tick��

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
