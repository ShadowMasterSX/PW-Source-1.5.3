#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "usermsg.h"
#include "public_quest.h"


public_quest::~public_quest()
{
	for(SCORE_MAP::iterator it = _score.begin(); it != _score.end(); ++it)
	{
		if(it->second) delete it->second;	
	}
}

void public_quest::RestartClear()
{
	//在外面已经加过锁了
	ASSERT(_lock);
	
	_pq_state = PQ_STATE_INITIALED;
	_heartbeat_counter = 0;
	_cur_child_task_id = 0;
	_cur_task_stamp = 0;
	_score_changed = false;
	_no_change_rank = false;
	for(SCORE_MAP::iterator it = _score.begin(); it != _score.end(); ++it)
	{
		if(it->second) delete it->second;	
	}
	_score.clear();
	for(CLS_RANKS_LIST::iterator it = _ranks.begin(); it!=_ranks.end(); ++it)
	{
		it->clear();	
	}
}

void public_quest::Init(int task_id, int child_task_id, int* common_value, int size)
{
	_task_id = task_id;
	_first_child_task_id = child_task_id;
	for(int i=0; i<size; i++)
	{
		_common_value.insert(std::pair<int,int>(common_value[i],0));
	}
	_pq_state = PQ_STATE_INITIALED;
}

void public_quest::Heartbeat(int tick)
{
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return;
	if(!_score.size()) return;
	_heartbeat_counter += tick;
	if(_heartbeat_counter%PQ_COMMONVALUE_NOTIFY_INTERVAL == 0)
	{
		BroadcastCommonvalue();	
	}
	if(_heartbeat_counter%PQ_RANKS_SORT_INTERVAL == 0)
	{
		if(_score_changed && !_no_change_rank) 
		{
			SortRanks();
			BroadcastRanks();
			_score_changed = false;
			BroadcastInfo();	//名次改变，更新信息
		}
	}
}

bool public_quest::SetStart(int* common_value, int size, bool no_change_rank)
{
	spin_autolock keeper(_lock);
	if(_pq_state == PQ_STATE_INVALID) return false;
	RestartClear();
	_cur_child_task_id = _first_child_task_id;
	_cur_task_stamp = time(NULL);
	for(int i=0; i<size; i++)	
	{
		//这里对全局变量的重置不需要通知客户端，因为此时没人接到任务
		std::map<int,int>::iterator it = _common_value.find(common_value[i]);
		if(it != _common_value.end())
		{
			it->second = 0;
			world_manager::GetInstance()->GetWorldByIndex(0)->SetCommonValue(it->first,0);
		}
	}
	_no_change_rank = no_change_rank;
	_pq_state = PQ_STATE_RUNNING;
	return true;
}

bool public_quest::SetFinish()
{
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return false;
	if(_score_changed) 
	{
		SortRanks();
		BroadcastRanks();
		_score_changed = false;
	}
	_cur_child_task_id = 0;
	_pq_state = PQ_STATE_FINISHED;
	BroadcastInfo();	//子任务改变，更新信息
	return true;
}

bool public_quest::SetNextChildTask(int child_task_id, int* common_value, int size, bool no_change_rank)
{
	spin_autolock keeper(_lock);		
	if(_pq_state != PQ_STATE_RUNNING) return false;
	if(_score_changed) 
	{
		SortRanks();
		BroadcastRanks();
		_score_changed = false;
	}
	_cur_child_task_id = child_task_id;
	for(int i=0; i<size; i++)	
	{
		//这里对全局变量的重置需要通知客户端,否则任务显示会出错
		std::map<int,int>::iterator it = _common_value.find(common_value[i]);
		if(it != _common_value.end())
		{
			world_manager::GetInstance()->GetWorldByIndex(0)->SetCommonValue(it->first,0);
		}
		BroadcastCommonvalue();
	}
	BroadcastInfo();	//子任务改变，更新信息
	_no_change_rank = no_change_rank;
	return true;
}

bool public_quest::SetRandContrib(int fixed_contrib, int max_rand_contrib, int lowest_contrib)
{
	spin_autolock keeper(_lock);		
	if(_pq_state != PQ_STATE_RUNNING) return false;
	for(SCORE_MAP::iterator it=_score.begin(); it !=_score.end(); ++it)
	{
		player_pq_info & info = *(it->second);
		if(info.score < lowest_contrib)
		{
			if(max_rand_contrib <= 0)
				info.rand = -1;
			else
				info.rand = 0;
			continue;
		}
		
		info.score += fixed_contrib;
		if(max_rand_contrib <= 0) 
			info.rand = -1;
		else
		{
			info.rand = abase::Rand(0,max_rand_contrib);
			info.score += info.rand;
		}
	}
	if(fixed_contrib || max_rand_contrib > 0) _score_changed = true;
	return true;
}

int public_quest::GetState()
{
	spin_autolock keeper(_lock);
	return _pq_state;	
}

int public_quest::GetSubTask() 
{
	spin_autolock keeper(_lock);
	return _cur_child_task_id;	
}

int public_quest::GetTaskStamp() 
{
	spin_autolock keeper(_lock);
	return _cur_task_stamp;	
}

int public_quest::GetContrib(int role_id) 
{
	spin_autolock keeper(_lock);
	SCORE_MAP::iterator it = _score.find(role_id);
	if(it != _score.end())
	{
		return it->second->score;
	}
	return 0;
}

int public_quest::GetAllPlace(int role_id) 
{
	spin_autolock keeper(_lock);
	SCORE_MAP::iterator it = _score.find(role_id);
	if(it != _score.end())
	{
		return it->second->all_place;
	}
	return -1;
}

int public_quest::GetClsPlace(int role_id) 
{
	spin_autolock keeper(_lock);
	SCORE_MAP::iterator it = _score.find(role_id);
	if(it != _score.end())
	{
		return it->second->cls_place;
	}
	return -1;
}

bool public_quest::AddPlayer(int roleid)
{
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return false;
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(pPlayer == NULL) return false;
	ASSERT(pPlayer->spinlock);	//调用到这里时player应该是已经上锁的
	
	SCORE_MAP::iterator it = _score.find(roleid);
	if(it != _score.end())
	{
		it->second->link_id = pPlayer->cs_index;
		it->second->link_sid = pPlayer->cs_sid;
		it->second->is_active = true;
	}
	else
	{
		player_pq_info * pinfo = new player_pq_info;
		if(pinfo == NULL) return false;
		pinfo->roleid = roleid;
		pinfo->link_id = pPlayer->cs_index;
		pinfo->link_sid = pPlayer->cs_sid;
		pinfo->is_active = true;
		pinfo->race = pPlayer->base_info.race;
		pinfo->level = pPlayer->base_info.level;
		pinfo->score = 0;
		pinfo->cls_place = -1;
		pinfo->all_place = -1;
		pinfo->rand = -1;
		_score.insert(SCORE_MAP::value_type(roleid,pinfo));	
		//更新全局变量
		SendCommonvalue(pinfo->link_id, pinfo->roleid, pinfo->link_sid);
		SendInfo(pinfo->link_id, pinfo->roleid, pinfo->link_sid, 0, -1, -1);
		SendRanks(pinfo->link_id, pinfo->roleid, pinfo->link_sid);
	}
	return true;
}

bool public_quest::RemovePlayer(int roleid)
{
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return false;
	SCORE_MAP::iterator it = _score.find(roleid);
	if(it != _score.end())
	{
		//先通知客户端界面显示
		player_pq_info & info = *(it->second);	
		SendEmptyInfo(info.link_id, info.roleid, info.link_sid);
		//在排行榜中查找并删除
		CLS_RANKS & cranks = _ranks[info.race&0x7FFFFFFF];
		for(CLS_RANKS::iterator it1=cranks.lower_bound(info.score); it1 != cranks.upper_bound(info.score); it1++)
		{
			if(roleid == it1->second->roleid)
			{
				cranks.erase(it1);
				break;
			}
		}
		CLS_RANKS & allranks = _ranks[USER_CLASS_COUNT];
		for(CLS_RANKS::iterator it1=allranks.lower_bound(info.score); it1 != allranks.upper_bound(info.score); it1++)
		{
			if(roleid == it1->second->roleid)
			{
				allranks.erase(it1);
				break;
			}
		}
		delete it->second;	
		_score.erase(it);
		return true;
	}
	return false;
}

bool public_quest::UpdatePlayerContrib(int roleid, int inc_contrib)
{
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return false;
	SCORE_MAP::iterator it = _score.find(roleid);
	if(it != _score.end())
	{
		player_pq_info & info = *(it->second);
		info.score += inc_contrib;	
		_score_changed = true;
		SendInfo(info.link_id, info.roleid, info.link_sid, info.score, info.cls_place, info.all_place);		//个人贡献度改变，更新信息
		return true;
	}	
	return false;
}

bool public_quest::ClearPlayerContrib()
{
	spin_autolock keeper(_lock);
	SCORE_MAP::iterator it = _score.begin();
	SCORE_MAP::iterator ite = _score.end();
	for( ; it!=ite; ++it)
	{
		it->second->score = 0;	
	}
	_score_changed = true;
	return true;
}

void public_quest::EnterWorldInit(int roleid)
{
	//玩家断线后又上线更新link_id 和link_sid
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return;
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(pPlayer == NULL) return;
	ASSERT(pPlayer->spinlock);	//调用到这里时player应该是已经上锁的
	
	SCORE_MAP::iterator it = _score.find(roleid);
	if(it != _score.end())
	{
		player_pq_info & info = *(it->second);
		//更新link
		info.link_id = pPlayer->cs_index;
		info.link_sid = pPlayer->cs_sid;
		info.is_active = true;
		//更新全局变量
		SendCommonvalue(info.link_id, info.roleid, info.link_sid);
		SendInfo(info.link_id, info.roleid, info.link_sid, info.score, info.cls_place, info.all_place);
		SendRanks(info.link_id, info.roleid, info.link_sid);
	}
}

void public_quest::LeaveWorld(int roleid)
{
	spin_autolock keeper(_lock);
	if(_pq_state != PQ_STATE_RUNNING) return;
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(pPlayer == NULL) return;
	ASSERT(pPlayer->spinlock);	//调用到这里时player应该是已经上锁的
	
	SCORE_MAP::iterator it = _score.find(roleid);
	if(it != _score.end())
	{
		player_pq_info & info = *(it->second);
		info.is_active = false;	
	}	
}

void public_quest::SortRanks()
{
	//在外面已经加过锁了
	ASSERT(_lock);
	//必须先清空排行榜
	for(CLS_RANKS_LIST::iterator it = _ranks.begin(); it!=_ranks.end(); ++it)
	{
		it->clear();	
	}
	
	for(SCORE_MAP::iterator it = _score.begin(); it != _score.end(); ++it)			
	{
		player_pq_info& info = *(it->second);
		info.cls_place = -1;	//清除排名信息
		info.all_place = -1;
		CLS_RANKS & cranks = _ranks[info.race&0x7FFFFFFF];
		cranks.insert(CLS_RANKS::value_type(info.score,&info));
		CLS_RANKS & allranks = _ranks[USER_CLASS_COUNT];
		allranks.insert(CLS_RANKS::value_type(info.score,&info));
	}

	for(int cls = 0; cls < USER_CLASS_COUNT+1; cls ++)
	{
		CLS_RANKS & cranks = _ranks[cls];
		int place = 1;
		for(CLS_RANKS::reverse_iterator it = cranks.rbegin(); it != cranks.rend(); it ++)
		{
			if(cls == USER_CLASS_COUNT)
				it->second->all_place = place;
			else
				it->second->cls_place = place;
			place ++;
		}
	}
}

void public_quest::BroadcastRanks()
{
	//在外面已经加过锁了
	ASSERT(_lock);
	//组织排行榜数据
	packet_wrapper h1(1024);
	using namespace S2C;
	CMD::Make<CMD::public_quest_ranks>::From(h1, _task_id, _score.size());
	//先组织所有排行榜大小
	for(int cls=0; cls<USER_CLASS_COUNT+1; cls++)
	{
		CLS_RANKS & cranks = _ranks[cls];
		CMD::Make<CMD::public_quest_ranks>::ClsRanksCount(h1, cranks.size()>PQ_RANKS_NOTIFY_CLIENT?PQ_RANKS_NOTIFY_CLIENT:cranks.size());
	}
	//再组织所有排行榜数据
	for(int cls=0; cls<USER_CLASS_COUNT+1; cls++)
	{
		CLS_RANKS & cranks = _ranks[cls];
		int count = 0;
		for(CLS_RANKS::reverse_iterator it=cranks.rbegin(); it!=cranks.rend()&&count<PQ_RANKS_NOTIFY_CLIENT; it++,count++)
		{
			player_pq_info& info = *(it->second);
			int place = (cls==USER_CLASS_COUNT?info.all_place:info.cls_place);
			CMD::Make<CMD::public_quest_ranks>::ClsRanksEntry(h1, info.roleid, info.race, info.score, info.rand,place);
		}
	}
	//发送
	typedef abase::vector<std::pair<int/*roleid*/,int/*sid*/>, abase::fast_alloc<> > cs_user_list;
	typedef abase::hash_map<int, cs_user_list, abase::_hash_function,abase::fast_alloc<> > cs_user_map;
	cs_user_map map;
	for(SCORE_MAP::iterator it = _score.begin(); it != _score.end(); ++it)
	{
		player_pq_info & info = *(it->second);
		if(info.is_active)
			map[info.link_id].push_back(std::pair<int,int>(info.roleid,info.link_sid));		
	}
	multi_send_ls_msg(map,h1);
}

void public_quest::BroadcastInfo()
{
	//在外面已经加过锁了
	ASSERT(_lock);
	packet_wrapper h1(32);
	using namespace S2C;
	for(SCORE_MAP::iterator it = _score.begin(); it != _score.end(); ++it)
	{
		h1.clear();
		player_pq_info & info = *(it->second);
		if(!info.is_active) continue;
		CMD::Make<CMD::public_quest_info>::From(h1, _task_id, _cur_child_task_id, info.score, info.cls_place, info.all_place);
		send_ls_msg(info.link_id, info.roleid, info.link_sid, h1);
	}
}

void public_quest::BroadcastCommonvalue()
{
	if(world_manager::GetWorldLimit().common_data_notify) return;
	//在外面已经加过锁了
	ASSERT(_lock);
	bool changed = false;
	packet_wrapper h1(64);
	using namespace S2C;
	CMD::Make<CMD::common_data_notify>::From(h1);
	for(std::map<int,int>::iterator it=_common_value.begin(); it!=_common_value.end(); it++)
	{
		int key = it->first;	
		int value = world_manager::GetInstance()->GetWorldByIndex(0)->GetCommonValue(key);
		if(value != it->second)
		{
			it->second = value;
			CMD::Make<CMD::common_data_notify>::Add(h1,key,value);
			changed = true;
		}
	}
	//发送
	if(changed)
	{
		typedef abase::vector<std::pair<int/*roleid*/,int/*sid*/>, abase::fast_alloc<> > cs_user_list;
		typedef abase::hash_map<int, cs_user_list, abase::_hash_function,abase::fast_alloc<> > cs_user_map;
		cs_user_map map;
		for(SCORE_MAP::iterator it = _score.begin(); it != _score.end(); ++it)
		{
			player_pq_info & info = *(it->second);
			if(info.is_active)
				map[info.link_id].push_back(std::pair<int,int>(info.roleid,info.link_sid));		
		}
		multi_send_ls_msg(map,h1);
	}
}

void public_quest::SendRanks(int link_id, int roleid, int link_sid)
{
	//在外面已经加过锁了
	ASSERT(_lock);
	//组织排行榜数据
	packet_wrapper h1(1024);
	using namespace S2C;
	CMD::Make<CMD::public_quest_ranks>::From(h1, _task_id, _score.size());
	//先组织所有排行榜大小
	for(int cls=0; cls<USER_CLASS_COUNT+1; cls++)
	{
		CLS_RANKS & cranks = _ranks[cls];
		CMD::Make<CMD::public_quest_ranks>::ClsRanksCount(h1, cranks.size()>PQ_RANKS_NOTIFY_CLIENT?PQ_RANKS_NOTIFY_CLIENT:cranks.size());
	}
	//再组织所有排行榜数据
	for(int cls=0; cls<USER_CLASS_COUNT+1; cls++)
	{
		CLS_RANKS & cranks = _ranks[cls];
		int count = 0;
		for(CLS_RANKS::reverse_iterator it=cranks.rbegin(); it!=cranks.rend()&&count<PQ_RANKS_NOTIFY_CLIENT; it++,count++)
		{
			player_pq_info& info = *(it->second);
			int place = (cls==USER_CLASS_COUNT?info.all_place:info.cls_place);
			CMD::Make<CMD::public_quest_ranks>::ClsRanksEntry(h1, info.roleid, info.race, info.score, info.rand,place);
		}
	}
	send_ls_msg(link_id, roleid, link_sid, h1);
}

void public_quest::SendInfo(int link_id, int roleid, int link_sid, int score, int cls_place, int all_place)
{
	packet_wrapper h1(32);
	using namespace S2C;
	CMD::Make<CMD::public_quest_info>::From(h1, _task_id, _cur_child_task_id, score, cls_place, all_place);
	send_ls_msg(link_id, roleid, link_sid, h1);
}

void public_quest::SendEmptyInfo(int link_id, int roleid, int link_sid)
{
	packet_wrapper h1(32);
	using namespace S2C;
	CMD::Make<CMD::public_quest_info>::From(h1, _task_id, 0, 0, -1, -1);
	send_ls_msg(link_id, roleid, link_sid, h1);
}

void public_quest::SendCommonvalue(int link_id, int roleid, int link_sid)
{
	if(world_manager::GetWorldLimit().common_data_notify) return;
	//在外面已经加过锁了
	ASSERT(_lock);
	packet_wrapper h1(128);
	using namespace S2C;
	CMD::Make<CMD::common_data_notify>::From(h1);
	for(std::map<int,int>::iterator it=_common_value.begin(); it!=_common_value.end(); it++)
	{
		CMD::Make<CMD::common_data_notify>::Add(h1,it->first,it->second);			
	}
	send_ls_msg(link_id, roleid, link_sid, h1);	
}
	
public_quest_manager::~public_quest_manager()
{
	for(PUBLIC_QUEST_MAP::iterator it = _pq_map.begin(); it != _pq_map.end(); ++ it)
	{
		if(it->second) delete it->second;	
	}
}

void public_quest_manager::Heartbeat(int tick)
{
	abase::vector<public_quest*, abase::fast_alloc<> > list;
	{
		spin_autolock keeper(_lock_map);
		for(PUBLIC_QUEST_MAP::iterator it = _pq_map.begin(); it != _pq_map.end(); ++ it)
		{
			list.push_back(it->second);
		}
	}
	for(size_t i=0; i<list.size(); i++)
	{
		list[i]->Heartbeat(tick);	
	}
}

public_quest * public_quest_manager::GetPublicQuest(int task_id)
{
	spin_autolock keeper(_lock_map);
	PUBLIC_QUEST_MAP::iterator it = _pq_map.find(task_id);	
	if(it != _pq_map.end())
		return it->second;
	else
		return NULL;
}

bool public_quest_manager::InitAddQuest(int task_id, int child_task_id, int* common_value, int size )
{
	ASSERT(world_manager::GetInstance()->IsUniqueWorld());

	spin_autolock keeper(_lock_map);
	PUBLIC_QUEST_MAP::iterator it = _pq_map.find(task_id);	
	if(it != _pq_map.end())
	{
		return false;	
	}
	else
	{
		public_quest * quest = new public_quest();
		if(quest == NULL) return false;
		quest->Init(task_id,child_task_id,common_value,size);
		_pq_map.insert(PUBLIC_QUEST_MAP::value_type(task_id,quest));
		return true;
	}
}

bool public_quest_manager::QuestSetStart(int task_id,int* common_value, int size, bool no_change_rank)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->SetStart(common_value,size,no_change_rank);
	}
	return false;
}

bool public_quest_manager::QuestSetFinish(int task_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->SetFinish();
	}
	return false;
}

bool public_quest_manager::QuestSetNextChildTask(int task_id, int child_task_id, int* common_value, int size, bool no_change_rank)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->SetNextChildTask(child_task_id,common_value,size,no_change_rank);
	}
	return false;
}

bool public_quest_manager::QuestSetRandContrib(int task_id, int fixed_contrib, int max_rand_contrib, int lowest_contrib)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->SetRandContrib(fixed_contrib,max_rand_contrib,lowest_contrib);
	}
	return false;
}

int public_quest_manager::GetQuestState(int task_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->GetState();
	}
	return public_quest::PQ_STATE_INVALID;	
}
	
int public_quest_manager::GetCurSubTask(int task_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->GetSubTask();
	}
	return 0;	
}

int public_quest_manager::GetCurTaskStamp(int task_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->GetTaskStamp();
	}
	return 0;	
}

int public_quest_manager::GetCurContrib(int task_id, int role_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->GetContrib(role_id);
	}
	return 0;	
}

int public_quest_manager::GetCurAllPlace(int task_id, int role_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->GetAllPlace(role_id);
	}
	return -1;	
}

int public_quest_manager::GetCurClsPlace(int task_id, int role_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->GetClsPlace(role_id);
	}
	return -1;	
}

bool public_quest_manager::QuestAddPlayer(int task_id, int role_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->AddPlayer(role_id);
	}
	return false;	
}

bool public_quest_manager::QuestRemovePlayer(int task_id, int role_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->RemovePlayer(role_id);
	}
	return false;		
}

bool public_quest_manager::QuestUpdatePlayerContrib(int task_id, int roleid, int inc_contrib)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		return quest->UpdatePlayerContrib(roleid,inc_contrib);
	}
	return false;		
}

void public_quest_manager::QuestEnterWorldInit(int task_id, int roleid)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		quest->EnterWorldInit(roleid);
	}
}

void public_quest_manager::QuestLeaveWorld(int task_id, int role_id)
{
	public_quest * quest = GetPublicQuest(task_id);
	if(quest != NULL)
	{
		quest->LeaveWorld(role_id);
	}
}

