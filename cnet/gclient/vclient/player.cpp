#include "dbgprt.h"
#include "protocol.h"
#include "../vclient_if.h"
#include "player.h"
#include "playermanager.h"
#include "msg.h"
#include "msgmanager.h"
#include "virtualclient.h"
#include "task.h"
#include "moveagent.h"
#include "templatedataman.h"
#include "property.h"



PlayerImp::PlayerImp(Player * _parent):parent(_parent),is_zombie(false),level(0),exp(0),
							hp(0),max_hp(0),mp(0),max_mp(0),
							attack_range(0.f),run_speed(0.f),
							money(0),money_capacity(200000000),
							pos(0.f,0.f,0.f),move_seq(0),is_moving(false),dir(0),
							cur_task(NULL),
							ai_mode(AI_MODE_IDLE),selected_target(-1),follow_target(-1),action_failed(0),force_attack(0)
{
}

PlayerImp::~PlayerImp()
{
}

void PlayerImp::Init()
{
}

void PlayerImp::Release()
{
	ClearTask();
	delete this;
}

void PlayerImp::Tick()
{
	if(rand()%4096 == 0) 
		__PRINTINFO("player %d curpos %f %f %f\n", parent->id, pos.x, pos.y, pos.z);
	
	AITick();
}

bool PlayerImp::AddTask(task_basic * task)
{
	if(task_list.size() >= MAX_PLAYER_TASK)
	{
		for(std::list<task_basic *>::iterator it=task_list.begin(); it!=task_list.end(); ++it)
		{
			if(*it != NULL) delete *it;
		}
		task_list.clear();
	}
	task_list.push_back(task);
	return !cur_task;
}

bool PlayerImp::StartTask()
{
	if(cur_task) return false;
	bool rst = false;
	while(!task_list.empty())
	{
		cur_task = task_list.front();
		task_list.pop_front();
		
		if(rst = cur_task->StartTask()) 
			break;
		else
			EndCurTask();
	}
	return rst;
}

bool PlayerImp::EndCurTask()
{
	if(!cur_task) return false;
	cur_task->EndTask();
	delete cur_task;
	cur_task = NULL;
	return true;
}

void PlayerImp::ClearTask()
{
	if(cur_task)
	{
		EndCurTask();
	}
	
	for(std::list<task_basic *>::iterator it=task_list.begin(); it!=task_list.end(); ++it)
	{
		if(*it != NULL) delete *it;
	}
	task_list.clear();
}

typedef void (PlayerImp::*AI_TICK_FUNC)();
static AI_TICK_FUNC ai_tick_func_table[PlayerImp::AI_MODE_MAX] = {	&PlayerImp::IdleModeTick,
												&PlayerImp::ZombieModeTick,
												&PlayerImp::DebugModeTick,
												&PlayerImp::PatrolModeTick,
												&PlayerImp::FollowModeTick,
												&PlayerImp::AutoModeTick};
void PlayerImp::AITick()
{
	if(cur_task)
	{
		if(!cur_task->Tick())
		{
			EndCurTask();
			StartTask();
		}
		return;
	}
	//
	(this->*ai_tick_func_table[ai_mode])();
}

void PlayerImp::SwitchAIMode(int mode)
{ 
	ai_mode = mode; 
	__PRINTF("Switch AI Mode: %d\n",mode);
}

int PlayerImp::GatherTarget(float range)
{
	enum{ TARGET_MAX = 3,};
	range *= range;
	struct{
		int id;
		float dis;
	}target_list[TARGET_MAX];
	int count = 0;
	
	int max_dis_index;
	float max_dis;
	float tmp_dis;
	std::map<int,object_info>::iterator it = object_list.begin();
	for( ; it!=object_list.end(); ++it)
	{
		object_info & info = it->second;
		if(IsPlayer(info.id))
		{
			if(!force_attack || !PlayerCanAttack(info.id)) continue;
		}
		else if(IsMatter(info.id))
		{
			size_t pile_limit;
			if(!MatterCanPickup(info.tid,pile_limit)) continue;
		}
		else if(IsNPC(info.id))
		{
			if(!NpcCanAttack(info.tid)) continue;
		}
		tmp_dis = pos.squared_distance(info.pos);
		if(tmp_dis > range) continue;
		
		if(count < TARGET_MAX)
		{
			target_list[count].id = info.id;
			target_list[count].dis = tmp_dis;
			++count;
		}
		else if(tmp_dis < max_dis)
		{
			target_list[max_dis_index].id = info.id;
			target_list[max_dis_index].dis = tmp_dis;
		}
		else
			continue;
		
		if(count == TARGET_MAX)
		{
			max_dis_index = -1;
			max_dis = -1.f;
			for(int i=0; i<TARGET_MAX; i++)
			{
				if(target_list[i].dis > max_dis)	
				{
					max_dis_index = i;
					max_dis = target_list[i].dis;
				}	
			}
		}
	}

	if(count == 0)
	{ 
		return -1;
	}
	else
	{
		int r = rand()%count;
		return target_list[r].id;
	}
}

bool PlayerImp::QueryTargetInfo(int id, object_info& info)
{
	std::map<int,object_info>::iterator it = object_list.find(id);
	if(it == object_list.end()) return false;
	info = it->second;
	return true;
}

int PlayerImp::DetermineAttackSkill(float & squared_range)
{
	if(rand()%3)
	{
		//66%几率使用技能
		if(attack_skills.size())
		{
			skill & sk = attack_skills[ rand()%attack_skills.size() ];
			if(CheckSkillCondition(sk))
			{
				squared_range = sk.range*sk.range;
				return sk.id;
			}
		}
	}
	//使用普通攻击
	squared_range = attack_range*attack_range;
	return 0;
}

bool PlayerImp::CheckSkillCondition(skill& sk)
{
	if(mp < sk.mp_cost) return false;
	return true;
}

void PlayerImp::IdleModeTick()
{
	AddTask(new task_idle(this, 1000));
	StartTask();
}

void PlayerImp::PatrolModeTick()
{
	A3DVECTOR next_pos;
	if(MoveAgentGetMovePos(pos,dir,run_speed,next_pos))
	{
		float distance = next_pos.squared_distance(pos);
		int use_time = int(sqrtf(distance)/run_speed*1000);
		if(use_time < 100) use_time = 100;
		if(use_time > 1000) use_time = 1000;
		AddTask(new task_move(this, next_pos, use_time, run_speed, 0x21));
		StartTask();
		is_moving = true;
	}
	else
	{
		if(is_moving)
		{
			AddTask(new task_stop_move(this, pos, 100, run_speed, 0x21));
			StartTask();
			is_moving = false;
		}
		if(rand()%3 == 0)
			dir = rand()%256;
	}

	if(!cur_task)
	{
		AddTask(new task_idle(this, 1000));
		StartTask();
	}
}

void PlayerImp::FollowModeTick()
{
	object_info info;
	if(!QueryTargetInfo(follow_target,info))
	{
		SwitchAIMode(AI_MODE_AUTO);
		return;
	}
	
	float min_dis = 5.f*5.f;
	if(info.pos.squared_distance(pos) <= min_dis) return;
	
	A3DVECTOR next_pos;
	if(MoveAgentGetMovePos(pos,info.pos,run_speed,next_pos))
	{
		float distance = next_pos.squared_distance(pos);
		int use_time = int(sqrtf(distance)/run_speed*1000);
		if(use_time < 100) use_time = 100;
		if(use_time > 1000) use_time = 1000;
		bool stop = next_pos.squared_distance(info.pos) < min_dis;
		if(stop)
		{
			AddTask(new task_stop_move(this, next_pos, use_time, run_speed, 0x21));
			is_moving = false;
		}
		else
		{
			AddTask(new task_move(this, next_pos, use_time, run_speed, 0x21));
			is_moving = true;
		}
		StartTask();
	}
	else
	{
		if(is_moving)
		{
			AddTask(new task_stop_move(this, pos, 100, run_speed, 0x21));
			StartTask();
			is_moving = false;
		}
	}

	if(!cur_task)
	{
		AddTask(new task_idle(this, 1000));
		StartTask();
	}
}

void PlayerImp::AutoModeTick()
{
	if(selected_target == -1)
	{
		int target_id = GatherTarget(50.f);
		if(target_id == -1)
		{
		}
		else if(IsPlayer(target_id) || IsNPC(target_id))
		{
			select_target(target_id);
		}
		else if(IsMatter(target_id))
		{
			selected_target = target_id;
			action_failed = 0;
			__PRINTF("select matter target %d\n", target_id);
		}
		if(is_moving)
		{
			AddTask(new task_stop_move(this, pos, 100, run_speed, 0x21));
			StartTask();
			is_moving = false;
		}
	}
	else
	{
		object_info info;
		if(QueryTargetInfo(selected_target,info))
		{
			float min_dis = 0.f;
			int use_skill = 0;
			if(IsPlayer(info.id) || IsNPC(info.id))
				use_skill = DetermineAttackSkill(min_dis);
			else if(IsMatter(info.id))
				min_dis = 9.5f*9.5f;

			if(pos.squared_distance(info.pos) < min_dis)
			{
				if(IsPlayer(info.id) || IsNPC(info.id))
				{
					if(use_skill) cast_skill(use_skill,force_attack,1,&info.id);
					else normal_attack(force_attack);
				}
				else if(IsMatter(info.id))
				{
					pickup(info.id,info.tid);
				}
			}
			else
			{
				A3DVECTOR next_pos;
				if(MoveAgentGetMovePos(pos,info.pos,run_speed,next_pos))
				{
					float distance = next_pos.squared_distance(pos);
					int use_time = int(sqrtf(distance)/run_speed*1000);
					if(use_time < 100) use_time = 100;
					if(use_time > 1000) use_time = 1000;
					bool stop = next_pos.squared_distance(info.pos) < min_dis;
					if(stop)
					{
						AddTask(new task_stop_move(this, next_pos, use_time, run_speed, 0x21));
						is_moving = false;
					}
					else
					{
						AddTask(new task_move(this, next_pos, use_time, run_speed, 0x21));
						is_moving = true;
					}
					StartTask();
				}
				else
				{
					if(is_moving)
					{
						AddTask(new task_stop_move(this, pos, 100, run_speed, 0x21));
						StartTask();
						is_moving = false;
					}
				}
			}

			if(!cur_task && ++action_failed == 3)
			{
				__PRINTF("action failed. target=%d\n",info.id);
				unselect();
			}
		}
		else
		{
			unselect();
			if(is_moving)
			{
				AddTask(new task_stop_move(this, pos, 100, run_speed, 0x21));
				StartTask();
				is_moving = false;
			}
		}
	}

	if(!cur_task)
	{
		AddTask(new task_idle(this, 1000));
		StartTask();
	}
}

void PlayerImp::DebugModeTick()
{
	if(level < 10)//level up
	{
		send_debug_cmd(10889,10000000);
		A3DVECTOR pos;
		pos.x = g_init_region.left + (g_init_region.right - g_init_region.left)*(rand()%100)/100;
		pos.z = g_init_region.top + (g_init_region.bottom - g_init_region.top)*(rand()%100)/100;
		send_debug_cmd(C2S::GOTO,&pos,sizeof(pos));
	}
	else if(hp < max_hp/2)
	{
		int index = GetItemIndex(inventory,27070);
		if(index == -1)
			send_debug_cmd(10800,27070);	
		else
			equip_item(index, 20);
	}
	else if(mp < max_mp/2)
	{
		int index = GetItemIndex(inventory,27071);
		if(index == -1)
			send_debug_cmd(10800,27071);	
		else
			equip_item(index, 21);
	}
	else if(!IsItemExist(equipment,11215,3))//穿上gm披风
	{
		int index = GetItemIndex(inventory,11215);
		if(index == -1)
		{
			send_debug_cmd(10800,11215);
		}
		else
		{
			equip_item(index, 3);
		}
	}
	else if(IsItemExist(equipment,2250,0))//把弓箭拿下来
	{
		int index = GetEmptySlot();
		if(index >= 0)
		{
			equip_item(index, 0);
		}
		else
		{
		}
	}
	else if(GetItemCount(inventory,3043) < 10)//复活卷轴
	{
		send_debug_cmd(10800,3043,10);
	}
	else if(true)// goto
	{
		A3DVECTOR pos;
		pos.x = g_init_region.left + (g_init_region.right - g_init_region.left)*(rand()%100)/100;
		pos.z = g_init_region.top + (g_init_region.bottom - g_init_region.top)*(rand()%100)/100;
		send_debug_cmd(C2S::GOTO,&pos,sizeof(pos));
		SwitchAIMode(AI_MODE_AUTO);
		__PRINTINFO("player %d jump curpos %f %f %f\n", parent->id, pos.x, pos.y, pos.z);
	}

	AddTask(new task_idle(this, 3000));
	StartTask();
}

void PlayerImp::ZombieModeTick()
{
	if(is_zombie)
	{
		int index = GetItemIndex(inventory,3043);
		//暂时不管冷却
		if(index != -1) resurrect_by_item();
		else resurrect_in_town();
		AddTask(new task_idle(this, 3000));
		StartTask();
	}
	else
	{
		SwitchAIMode(AI_MODE_DEBUG);
	}
}

bool PlayerImp::MatterCanPickup(int tid, size_t & pile_limit)
{
	if(tid == 3044)
	{
		if(money >= money_capacity) return false;
	}
	else
	{
		int empty_index = -1;
		for(int i=0; i<inventory.size(); i++)
		{
			if(inventory[i].type == -1)
			{
				empty_index = i;
				break;
			}
		}
		if(empty_index == -1) return false;
	}
	return g_virtualclient.GetTemplateDataMan()->MatterCanPickup(tid,pile_limit);
}

bool PlayerImp::MatterCanGather(int tid)
{
	return g_virtualclient.GetTemplateDataMan()->MatterCanGather(tid);
}

bool PlayerImp::NpcCanAttack(int tid)
{
	return g_virtualclient.GetTemplateDataMan()->NpcCanAttack(tid);
}

bool PlayerImp::PlayerCanAttack(int id)
{
	return g_virtualclient.GetPlayerManager()->FindPlayer(id) < 0;
}

bool PlayerImp::IsAttackSkill(int id, int & mp_cost, float & range)
{
	return g_virtualclient.GetTemplateDataMan()->IsAttackSkill(id,mp_cost,range);
}

size_t PlayerImp::GetItemCount(std::vector<item>& inv, int type)
{
	size_t count = 0;
	for(size_t i=0; i<inv.size(); i++)
	{
		if(inv[i].type == type) count += inv[i].count;
	}
	return count;
}

int PlayerImp::GetItemIndex(std::vector<item>& inv, int type)	
{
	for(int i=0; i<inv.size(); i++)
	{
		if(inv[i].type == type) return i; 
	}
	return -1;
}

int PlayerImp::PushItem(int type, size_t & count, size_t pile_limit)
{
	ASSERT(count <= pile_limit);
	size_t oldcount = count;
	int empty_index = -1;
	if(pile_limit > 1)
	{
		int last_index = -1;
		for(size_t i=0; i<inventory.size(); i++)
		{
			item & it = inventory[i];
			if(it.type == -1)
			{
				if(empty_index == -1) empty_index = i;
			}
			else if(it.type == type && it.count < pile_limit)
			{
				if(count <= pile_limit - it.count)
				{
					//全部堆叠成功
					it.count += count;
					count = 0;
					return i;
				}
				else
				{
					count -= pile_limit - it.count;
					it.count = pile_limit;
					last_index = i;
				}
			}		
		}
		//一个都未堆叠或部分堆叠
		if(empty_index == -1)
		{
			if(oldcount == count) return -1;	//一个都没放进去
			else return last_index;				//放进一些
		}
	}
	else
	{
		for(size_t i=0; i<inventory.size(); i++)
		{
			item & it = inventory[i];
			if(it.type == -1)
			{
				empty_index = i;
				break;
			}
		}
		if(empty_index == -1) return -1;
	}
	
	inventory[empty_index].type = type;
	inventory[empty_index].count = count;
	count = 0;
	return empty_index;
}

int PlayerImp::GetEmptySlot()
{
	for(int i=0; i<inventory.size(); i++)
	{
		if(inventory[i].type == -1) return i;
	}
	return -1;
}

void PlayerImp::MoveTo(A3DVECTOR & dest)
{ 
	//方向由移动方向决定
	A3DVECTOR direction = dest;
	direction -= pos;
	if(direction.squared_magnitude() > 1e-6) dir = DirConvert(direction);
	pos = dest; 
}

void PlayerImp::MoveAgentLearn(const A3DVECTOR & pos)
{
	g_virtualclient.GetMoveAgent()->Learn(pos);
}

bool PlayerImp::MoveAgentGetMovePos(const A3DVECTOR & cur_pos, const A3DVECTOR & dest_pos, float range, A3DVECTOR & next_pos)
{
	return g_virtualclient.GetMoveAgent()->GetMovePos(cur_pos,dest_pos,range,next_pos);
}

bool PlayerImp::MoveAgentGetMovePos(const A3DVECTOR & cur_pos, int dir, float range, A3DVECTOR & next_pos)
{
	return g_virtualclient.GetMoveAgent()->GetMovePos(cur_pos,dir,range,next_pos);
}

//MSG * msg = BuildMessage(MSG_TEST,parent->id,123);
//SendMessage(msg);
void PlayerImp::SendMessage(MSG * msg)
{
	g_virtualclient.GetMsgManager()->PostMessage(msg);
}

void PlayerImp::SendC2SGameData(void * buf, size_t size)
{
	VCLIENT::SendC2SGameData(parent->id,buf,size);
}

#pragma pack(1)
struct DEBUG_CMD
{
	short cmd;
	int param[5];
};
#pragma pack()
void PlayerImp::send_debug_cmd(short cmd, int param)
{
	DEBUG_CMD data;
	data.cmd = cmd;
	data.param[0] = param;
	SendC2SGameData(&data,sizeof(short)+sizeof(int));
}

void PlayerImp::send_debug_cmd(short cmd, int param1, int param2)
{
	DEBUG_CMD data;
	data.cmd = cmd;
	data.param[0] = param1;
	data.param[1] = param2;
	SendC2SGameData(&data,sizeof(short)+sizeof(int)*2);
}

void PlayerImp::send_debug_cmd(short cmd, void * buf, size_t size)
{
	if(size <= sizeof(int)*5)
	{
		DEBUG_CMD data;
		data.cmd = cmd;
		memcpy(data.param, buf, size);
		SendC2SGameData(&data,sizeof(short)+size);
	}
}

void PlayerImp::get_all_data()
{
	C2S::CMD::get_all_data data;
	data.header.cmd 	= C2S::GET_ALL_DATA;
	data.detail_inv 	= 1;
	data.detail_equip 	= 1;
	data.detail_task 	= 0;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::player_move(A3DVECTOR & pos, int time, float speed, char mode)
{
	C2S::CMD::player_move data;
	data.header.cmd = C2S::PLAYER_MOVE;
	data.info.cur_pos = pos;
	data.info.next_pos = pos;
	data.info.use_time = time;
	data.info.speed = (unsigned short)(speed*256.f);
	data.info.move_mode = mode;
	data.cmd_seq = move_seq;	//
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::player_stop_move(A3DVECTOR & pos, int time, float speed, char mode)
{
	C2S::CMD::player_stop_move data;
	data.header.cmd = C2S::STOP_MOVE;
	data.pos = pos;
	data.speed = (unsigned short)(speed*256.f);
	data.move_mode = mode;
	data.cmd_seq = move_seq;	//
	data.dir = dir;				//
	data.use_time = time;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::select_target(int id)
{
	C2S::CMD::select_target data;
	data.header.cmd = C2S::SELECT_TARGET;
	data.id = id;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::unselect()
{
	C2S::cmd_header data;
	data.cmd = C2S::UNSELECT;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::normal_attack(char force_attack)
{
	C2S::CMD::normal_attack data;
	data.header.cmd = C2S::NORMAL_ATTACK;
	data.force_attack = force_attack;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::cast_skill(int skill_id, char force_attack, size_t target_count, int* targets)
{
	union{
		C2S::CMD::cast_skill data;
		char buf[20];
	}data;
	data.data.header.cmd = C2S::CAST_SKILL;
	data.data.skill_id = skill_id;
	data.data.force_attack = force_attack;
	data.data.target_count = target_count;
	if(target_count)
		memcpy(data.data.targets,targets,target_count*sizeof(int));
	SendC2SGameData(&data.data,sizeof(data.data)+target_count*sizeof(int));
}

void PlayerImp::pickup(int id, int tid)
{
	C2S::CMD::pickup_matter data;
	data.header.cmd = C2S::PICKUP;
	data.mid = id;
	data.type = tid;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::equip_item(unsigned char inv_index, unsigned char eq_index)
{
	C2S::CMD::equip_item data;
	data.header.cmd = C2S::EQUIP_ITEM;
	data.idx_inv= inv_index;
	data.idx_eq= eq_index;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::get_inventory_detail(unsigned char where)
{
	C2S::CMD::get_inventory_detail data;
	data.header.cmd = C2S::GET_INVENTORY_DETAIL;
	data.where= where;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::resurrect_by_item()
{
	C2S::cmd_header data;
	data.cmd = C2S::RESURRECT_BY_ITEM;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::resurrect_in_town()
{
	C2S::cmd_header data;
	data.cmd = C2S::RESURRECT_IN_TOWN;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::check_security_passwd()
{
	C2S::CMD::check_security_passwd data;
	data.header.cmd = C2S::CHECK_SECURITY_PASSWD;
	data.passwd_size = 0;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::cancel_action()
{
	C2S::CMD::cancel_action data;
	data.header.cmd = C2S::CANCEL_ACTION;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::logout()
{
	C2S::CMD::logout data;
	data.header.cmd = C2S::LOGOUT;
	data.logout_type = 0;
	SendC2SGameData(&data,sizeof(data));
}

void PlayerImp::HandleMessage(MSG * msg)
{
	switch(msg->message)
	{
		case MSG_TEST:
			__PRINTF("player %d receive MSG_TEST\n",parent->id);
		break;

		case MSG_SWITCH_MODE:
		{
			int mode = msg->param;
			if(ai_mode >= AI_MODE_PATROL && mode >= AI_MODE_PATROL && mode < AI_MODE_MAX)
			{
				cancel_action();
				SwitchAIMode(mode);
			}
		}
		break;
			
		case MSG_FOLLOW_TARGET:
		{
			int target = msg->param;
			cancel_action();
			SwitchAIMode(AI_MODE_FOLLOW);
			follow_target = target;
		}
		break;
		
		case MSG_DEBUG_C2SCMD:
		{
			send_debug_cmd(msg->param,msg->content,msg->content_length);
		}
		break;
		
		case MSG_CHANGE_FORCE_ATTACK:
		{
			force_attack = msg->param & C2S::FORCE_ATTACK_MASK_ALL;	
			if(force_attack) force_attack |= C2S::FORCE_ATTACK;
			unselect();
			cancel_action();
		}
		break;
		
		default:
			__PRINTINFO("unknown msg. %d\n",msg->message);
		break;
	}
}

#define SIMPLE_CHECK_SIZE(cmd_struct)\
		cmd_struct & cmd = *(cmd_struct *)buf;\
		if(size < sizeof(cmd)) break;
void PlayerImp::HandleS2CCmd(void * buf, size_t size)
{
	if(size < sizeof(unsigned short)) return;

	switch(*(unsigned short*)buf)
	{
		//---------------------------------属性
		case S2C::SELF_INFO_1:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_info_1)
			exp = cmd.info.exp;
			ResetPos(cmd.info.pos);
			ResetMoveSeq(0);
			if(cmd.info.object_state & STATE_ZOMBIE) is_zombie = true;
			//标志着EnterWorld成功了
		//	get_all_data();
			AddTask(new task_load(this,3));// delay 3 tick
			StartTask();
		}
		break;

		case S2C::TASK_DATA:
		{
			//标志着数据全部获取完成,进入了世界
			VCLIENT::OnEnterWorld(parent->id);
			send_debug_cmd(8903,73125); // set gs player no cooldown mode used in COOLDOWN_INDEX_ sys
			SwitchAIMode(AI_MODE_ZOMBIE); // may already deaded or not  zombie-mode will switch auto-mode
		}
		break;

		case S2C::SELF_GET_EXT_PROPERTY:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_get_property)
			extend_prop & prop = *(extend_prop*)((char*)buf+sizeof(S2C::CMD::self_get_property));
			run_speed = prop.run_speed;
			attack_range = prop.attack_range;
		}
		break;
		
		case S2C::SELF_INFO_00:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_info_00)
			level 	= cmd.info.lvl;
			hp 		= cmd.info.hp;
			max_hp 	= cmd.info.max_hp;
			mp 		= cmd.info.mp;
			max_mp 	= cmd.info.max_mp;
		}
		break;
		
		case S2C::PLAYER_EXTPROP_MOVE:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_extprop_move)
			if(parent->id == cmd.id)
			{
				run_speed = cmd.run_speed;
			}
		}
		break;

		case S2C::NPC_INFO_00:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::npc_info_00)
			object_list[cmd.nid].hp = cmd.info.hp;
		}
		break;

		//----------------------------------物品
		case S2C::SELF_INVENTORY_DETAIL_DATA:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_inventory_detail_data)
			if(size != sizeof(S2C::CMD::self_inventory_detail_data) + cmd.content_length
					|| cmd.content_length < sizeof(size_t)) break;

			std::vector<item> * plist;
			if(cmd.where == IL_INVENTORY)	plist = &inventory;
			else if(cmd.where == IL_EQUIPMENT)	plist = &equipment;
			else break;

			(*plist).clear();
			(*plist).resize(cmd.inv_size);

			char * end = (char *)buf + size;
			char * p = (char *)cmd.content; 
			size_t count = *(size_t *)p; 	p += sizeof(size_t);
			bool err = false;
			size_t index;
			item it;
			unsigned short crc, len;
			for(size_t i=0; i<count; i++)
			{
				index = *(size_t *)p; 	p += sizeof(size_t);
				it.type = *(int *)p;	p += sizeof(int);
				p += sizeof(int);
				p += sizeof(int);
				it.count = *(size_t *)p;	p += sizeof(size_t);
				crc = *(unsigned short *)p;	p += sizeof(unsigned short);
				len = *(unsigned short *)p;	p += sizeof(unsigned short);
				if(len) p += len;
				if(p > end)
				{
					err = true;
					break;
				}
				if(index >= cmd.inv_size)
				{
					err = true;
					break;
				}
				(*plist)[index] = it;
			}
			if(err)
			{
				__PRINTINFO("SELF_INVENTORY_DETAIL_DATA size error\n");
				(*plist).clear();
			}
		}
		break;
		
		case S2C::GET_OWN_MONEY:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::get_own_money);
			money = cmd.amount;
			money_capacity = cmd.capacity;
		}
		break;

		case S2C::PICKUP_MONEY:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_pickup_money);
			money += cmd.amount;
		}
		break;

		case S2C::PICKUP_ITEM:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_pickup_item)
			if(cmd.where != IL_INVENTORY) break;
			size_t pile_limit;
			ASSERT(MatterCanPickup(cmd.type, pile_limit));

			size_t count = cmd.amount;
			int rst = PushItem(cmd.type, count, pile_limit);
			if(rst != cmd.index || inventory[rst].count != cmd.slot_amount || count != 0)
			{
				//与服务器堆叠结果不一致　重新获取一下包裹
				__PRINTINFO("\tERROR PushItem 与服务器实现不一致\n");
				get_inventory_detail(IL_INVENTORY);
			}
		}
		break;

		case S2C::EQUIP_ITEM:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::equip_item);
			if(cmd.index_inv >= inventory.size() || cmd.index_equip >= equipment.size()) break;

			std::swap(inventory[cmd.index_inv],equipment[cmd.index_equip]);
			if(inventory[cmd.index_inv].count != cmd.count_inv || equipment[cmd.index_equip].count != cmd.count_equip)
			{
				//与服务器堆叠结果不一致　重新获取一下包裹
				__PRINTINFO("\tERROR EQUIP_ITEM 结果与服务器不一致\n");
				get_inventory_detail(IL_INVENTORY);
				get_inventory_detail(IL_EQUIPMENT);
			}
		}
		break;
		
		case S2C::PLAYER_DROP_ITEM:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_drop_item);
			std::vector<item> * plist = NULL;
			if(cmd.where == IL_INVENTORY) plist = &inventory;
			else if(cmd.where == IL_EQUIPMENT) plist = &equipment;
			else break;
			if(cmd.index >= (*plist).size()) break;
			item & it = (*plist)[cmd.index];
			if(it.type != cmd.type) break;
			if(it.count > cmd.count)
				it.count -= cmd.count;
			else
			{
				it.count = 0;
				it.type = -1;
			}
		}
		break;
		
		case S2C::TRASHBOX_PASSWD_STATE:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::trashbox_passwd_state);
			if(!cmd.has_passwd) check_security_passwd(); 
		}
		break;

		//----------------------------------技能
		case S2C::SKILL_DATA:
		{
			S2C::CMD::skill_data & cmd = *(S2C::CMD::skill_data*)buf;
			if(size < sizeof(S2C::CMD::skill_data)+sizeof(size_t)) break;
			size_t count = *(size_t*)cmd.content;
			if(size != sizeof(S2C::CMD::skill_data)+sizeof(size_t)+count*(sizeof(short)+sizeof(char)+sizeof(short))) break;

			char * p = cmd.content + sizeof(size_t);
			skill sk;
			for(size_t i=0; i<count; i++)
			{
				sk.id = *(short*)p; p+= sizeof(short);
				sk.level = *(char*)p; p+= sizeof(char);
				p+= sizeof(short);

				if(IsAttackSkill(sk.id, sk.mp_cost, sk.range))
				{
					if(sk.range < 1e-6) sk.range = attack_range;
					attack_skills.push_back(sk);
				}			
			}		
		}
		break;

		//----------------------------------移动
		case S2C::OBJECT_MOVE:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::object_move)
			object_list[cmd.data.cid].pos = cmd.data.dest;
			if(cmd.data.move_mode & (C2S::MOVE_MASK_SKY|C2S::MOVE_MASK_WATER)) break;
			unsigned char mode = cmd.data.move_mode & 0x0F;
			if(mode != C2S::MOVE_MODE_WALK && mode != C2S::MOVE_MODE_RUN) break;
			MoveAgentLearn(cmd.data.dest);
		}
		break;

		case S2C::OBJECT_STOP_MOVE:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::object_stop_move)
			object_list[cmd.id].pos = cmd.pos;
			if(cmd.move_mode & (C2S::MOVE_MASK_SKY|C2S::MOVE_MASK_WATER)) break;
			unsigned char mode = cmd.move_mode & 0x0F;
			if(mode != C2S::MOVE_MODE_WALK && mode != C2S::MOVE_MODE_RUN) break;
			MoveAgentLearn(cmd.pos);
		}
		break;

		case S2C::SELF_TRACE_CUR_POS:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_trace_cur_pos)
			ResetPos(cmd.pos);
			ResetMoveSeq(cmd.seq);
			ClearTask();		//$$$$$$$$$$$$$$$
		}
		break;

		case S2C::CHANGE_MOVE_SEQ:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::change_move_seq);
			ResetMoveSeq(cmd.seq);
		}
		break;

		case S2C::OBJECT_NOTIFY_POS:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::notify_pos);
			//忽略cmd.tag
			ResetPos(cmd.pos);
			ClearTask();		//$$$$$$$$$$$$$$$
		}
		break;
		//----------------------------------对象出没	
		case S2C::PLAYER_ENTER_WORLD:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_enter_world)
			if(cmd.info.cid != parent->id)
			{
				object_info info;
				MakeObjectInfo(cmd.info, info);
				object_list[info.id] = info;
				__PRINTF("player %d enter world. \n",info.id);
			}
		}
		break;

		case S2C::NPC_ENTER_WORLD:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::npc_enter_world)
			object_info info;
			MakeObjectInfo(cmd.info, info);
			object_list[info.id] = info;
			__PRINTF("npc %d enter world. \n",info.id);
		}
		break;
		
		case S2C::MATTER_ENTER_WORLD:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::matter_enter_world)
			cmd.data.tid &= 0xFFFF;		//去掉物品的标志
			object_info info;
			MakeObjectInfo(cmd.data, info);
			object_list[info.id] = info;
			__PRINTF("matter %d enter world. \n",info.id);
			size_t pile_limit;
			if(pos.squared_distance(info.pos) <= 9.5f*9.5f
					&& MatterCanPickup(info.tid,pile_limit))
				pickup(info.id,info.tid);
		}
		break;

		case S2C::PLAYER_LEAVE_WORLD:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_leave_world)
			object_list.erase(cmd.cid);
			__PRINTF("player %d leave world. \n",cmd.cid);
		}
		break;
		
		case S2C::OBJECT_DISAPPEAR:	//玩家 npc尸体 matter
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::object_disappear)
			object_list.erase(cmd.nid);
			__PRINTF("object %d disappear. \n",cmd.nid);
		}
		break;

		case S2C::NPC_DEAD:		//死后无尸体
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::npc_dead)
			object_list.erase(cmd.nid);
			__PRINTF("npc %d dead. \n",cmd.nid);
		}
		break;
		
		case S2C::NPC_DEAD_2:	//死后有尸体
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::npc_dead_2)
			object_list.erase(cmd.nid);
			__PRINTF("npc %d dead, leave corpse. \n",cmd.nid);
		}
		break;
			
		case S2C::MATTER_PICKUP:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::matter_pickup)
			object_list.erase(cmd.matter_id);
			__PRINTF("matter %d picked up. \n",cmd.matter_id);
		}
		break;
		
		case S2C::PLAYER_ENTER_SLICE:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_enter_slice)
			if(cmd.data.cid != parent->id)
			{
				object_info info;
				MakeObjectInfo(cmd.data, info);
				object_list[info.id] = info;
				__PRINTF("player %d enter slice. \n",info.id);
			}
		}
		break;

		case S2C::NPC_ENTER_SLICE:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::npc_enter_slice)
			object_info info;
			MakeObjectInfo(cmd.data, info);
			object_list[info.id] = info;
				__PRINTF("npc %d enter slice. \n",info.id);
		}
		break;

		case S2C::OBJECT_LEAVE_SLICE:	//玩家或npc
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::leave_slice)
			object_list.erase(cmd.id);
				__PRINTF("object %d leave slice. \n",cmd.id);
		}
		break;
		
		case S2C::PLAYER_INFO_1_LIST:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_info_1_list)
			char * end = (char *)buf + size;
			char * p = (char *)cmd.list; 
			for(size_t i=0; i<cmd.header.count; i++)
			{
				S2C::INFO::player_info_1 & p_info = *(S2C::INFO::player_info_1 *)p;
				p += sizeof(S2C::INFO::player_info_1);
				if(p_info.object_state & STATE_ADV_MODE) 		p += 2*sizeof(int);
				if(p_info.object_state & STATE_SHAPE) 			p+= sizeof(unsigned char);
				if(p_info.object_state & STATE_EMOTE) 			p+= sizeof(unsigned char);
				if(p_info.object_state & STATE_EXTEND_PROPERTY) p+= 6*sizeof(unsigned int);
				if(p_info.object_state & STATE_MAFIA) 			p+= sizeof(int)+sizeof(unsigned char);
				if(p_info.object_state & STATE_MARKET) 			p+= sizeof(unsigned char);
				if(p_info.object_state & STATE_EFFECT)
				{
					unsigned char count = *(unsigned char*)p;
					p+= sizeof(unsigned char) + count * sizeof(short);
				}
				if(p_info.object_state & STATE_PARIAH) 			p+= sizeof(char);
				if(p_info.object_state & STATE_MOUNT) 			p+= sizeof(unsigned short)+sizeof(int);
				if(p_info.object_state & STATE_IN_BIND) 		p+= sizeof(char)+sizeof(int);
				if(p_info.object_state & STATE_SPOUSE) 			p+= sizeof(int);
				if(p_info.object_state & STATE_EQUIPDISABLED) 	p+= sizeof(int);
				if(p_info.object_state & STATE_PLAYERFORCE) 	p+= sizeof(int);
				if(p_info.object_state & STATE_MULTIOBJ_EFFECT)
				{
					int count = *(int*)p;	p+= sizeof(int);
					if(count > 0) p+= count*(sizeof(int) + sizeof(char));
				}
				if(p_info.object_state & STATE_COUNTRY) 		p+= sizeof(int);
				if(p_info.object_state2 & STATE_TITLE) 			p+= sizeof(unsigned short);
				if(p_info.object_state2 & STATE_REINCARNATION) 			p+= sizeof(unsigned char);
				if(p_info.object_state2 & STATE_REALMLEVEL) 			p+= sizeof(unsigned char);
				if(p_info.object_state2 & STATE_MAFIA_PVP_MASK)	p+= sizeof(unsigned char);
				if(p > end)
				{
					__PRINTINFO("PLAYER_INFO_1_LIST size error\n");
					break;
				}
				if(p_info.cid != parent->id)
				{
					object_info info;
					MakeObjectInfo(p_info, info);
					object_list[info.id] = info;
					__PRINTF("player %d info 1 list. \n",info.id);
				}			
			}	
		}
		break;

		case S2C::NPC_INFO_LIST:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::npc_info_list)
			char * end = (char *)buf + size;
			char * p = (char *)cmd.list; 
			for(size_t i=0; i<cmd.header.count; i++)
			{
				S2C::INFO::npc_info & n_info = *(S2C::INFO::npc_info*)p; 
				p += sizeof(S2C::INFO::npc_info); 
				if(n_info.object_state & STATE_EXTEND_PROPERTY) p+= 6*sizeof(int);
				if(n_info.object_state & STATE_NPC_PET) p+= sizeof(int);
				if(n_info.object_state & STATE_NPC_NAME)
				{
					unsigned char name_size = *(unsigned char*)p;
					p+= sizeof(unsigned char) + name_size*sizeof(char);
				}
				if(n_info.object_state & STATE_MULTIOBJ_EFFECT)
				{
					int count = *(int*)p;	p+= sizeof(int);
					if(count > 0) p+= count*(sizeof(int) + sizeof(char));
				}
				if(n_info.object_state & STATE_NPC_MAFIA)
				{
					p+= sizeof(int);	
				}
				if(p > end)
				{
					__PRINTINFO("NPC_INFO_LIST size error\n");
					break;
				}
				object_info info;
				MakeObjectInfo(cmd.list[i], info);
				object_list[info.id] = info;
				__PRINTF("npc %d info list. \n",info.id);
			}
		}
		break;
		
		case S2C::MATTER_INFO_LIST:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::matter_info_list)
			for(size_t i=0; i<cmd.header.count; i++)
			{
				cmd.list[i].tid &= 0xFFFF;		//去掉物品的标志
				object_info info;
				MakeObjectInfo(cmd.list[i], info);
				object_list[info.id] = info;
					__PRINTF("matter %d info  list. \n",info.id);
			}
		}
		break;

		case S2C::OUT_OF_SIGHT_LIST:	//玩家npc matter
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::OOS_list);
			for(size_t i=0; i<cmd.count; i++)
			{
				object_list.erase(cmd.id_list[i]);
					__PRINTF("object %d out of sight. \n",cmd.id_list[i]);
			}
		}
		break;
		//------------------------------------------攻击	
		case S2C::SELECT_TARGET:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_select_target)	
			selected_target = cmd.id;
			action_failed = 0;
			__PRINTF("select target %d\n", cmd.id);
		}
		break;

		case S2C::UNSELECT:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::unselect)
			__PRINTF("unselect target %d\n", selected_target);
			selected_target = -1;
		}
		break;

		case S2C::SELF_START_ATTACK:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_start_attack)
			AddTask(new task_normal_attack(this));
			StartTask();
		}
		break;
		
		case S2C::SELF_STOP_ATTACK:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_stop_attack)
			if(cur_task && cur_task->GetTaskType()==TASK_TYPE_NORMAL_ATTACK)
			{
				EndCurTask();
				StartTask();
			}
			else
				__PRINTINFO("\tERROR cur_task != TASK_TYPE_NORMAL_ATTACK\n");
		}
		break;
	
		case S2C::OBJECT_CAST_SKILL:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::object_cast_skill);
			if(cmd.caster == parent->id)
			{
				AddTask(new task_cast_skill(this));
				StartTask();
			}
		}
		break;

		case S2C::SELF_STOP_SKILL:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::self_stop_skill);
			if(cur_task && cur_task->GetTaskType()==TASK_TYPE_CAST_SKILL)
			{
				EndCurTask();
				StartTask();
			}
			else
				__PRINTINFO("\tERROR cur_task != TASK_TYPE_CAST_SKILL\n");
		}
		break;
		
		case S2C::ENCHANT_RESULT:
		{
		}
		break;

		case S2C::BE_KILLED:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::be_killed);
			is_zombie = true;
			SwitchAIMode(AI_MODE_ZOMBIE);
			ResetPos(cmd.pos);
			ClearTask();
		}
		break;

		case S2C::PLAYER_DEAD:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_dead);
			object_list[cmd.player].zombie = true;
		}
		break;
		
		case S2C::PLAYER_REVIVAL:
		{
			SIMPLE_CHECK_SIZE(S2C::CMD::player_revival);
			if(cmd.id == parent->id)
			{
				if(cmd.type == 0 || cmd.type == 2)
				{
					is_zombie = false;
					ResetPos(cmd.pos);
					ClearTask();
				}
			}
			else
			{
				if(cmd.type == 0 || cmd.type == 2)
				{
					object_list[cmd.id].zombie = false;
				}
			}
		}
		break;
		
		case S2C::BE_HURT:
		case S2C::COOLDOWN_DATA:

		break;
		
		//ignore cmd
		case S2C::SERVER_CONFIG_DATA:
		case S2C::COMMON_DATA_LIST:
		case S2C::WORLD_LIFE_TIME:
		case S2C::SERVER_TIMESTAMP:
		
		case S2C::NOTIFY_SAFE_LOCK:
		case S2C::PLAYER_REPUTATION:
		case S2C::PLAYER_WAYPOINT_LIST:
		case S2C::PLAYER_PET_ROOM_CAPACITY:
		case S2C::PLAYER_PET_ROOM:
		case S2C::PLAYER_PVP_MODE:
		case S2C::PLAYER_PVP_COOLDOWN:
		case S2C::PARIAH_DURATION:
		case S2C::PLAYER_CASH:
		case S2C::PLAYER_DIVIDEND:
		case S2C::DOUBLE_EXP_TIME:
		case S2C::AVAILABLE_DOUBLE_EXP_TIME:
		case S2C::MULTI_EXP_INFO:
		case S2C::CHANGE_MULTI_EXP_STATE:
		case S2C::FACTION_CONTRIB_NOTIFY:
		case S2C::FACTION_RELATION_NOTIFY:
		case S2C::ELF_VIGOR:
		case S2C::ELF_ENHANCE:
		case S2C::OBJECT_ENTER_SANCTUARY:
		case S2C::OBJECT_LEAVE_SANCTUARY:

		case S2C::PLAYER_CHANGE_SHAPE:
		break;
		//ignore cmd
		
		default:
			__PRINTF("S2C Cmd(cmd=%d size=%u) unhandled!\n", *(short*)buf, size);
		break;
	}
}
#undef SIMPLE_CHECK_SIZE 
