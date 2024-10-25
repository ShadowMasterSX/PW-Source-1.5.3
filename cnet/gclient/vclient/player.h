#ifndef __VCLIENT_PLAYER_H__
#define __VCLIENT_PLAYER_H__

#include <list>
#include <map>
#include <vector>
#include "spinlock.h"
#include "types.h"
#include "protocol.h"

class MSG;
class task_basic;
class PlayerImp;

enum OBJECT_STATE
{
		STATE_SHAPE		= 0x00000001,   //是否变身状态
		STATE_EMOTE		= 0x00000002,   //是否正在做表情
		STATE_INVADER 		= 0x00000004,   //是否粉名
		STATE_PARIAH 		= 0x00000008,   //是否红名
		STATE_FLY		= 0x00000010,   //是否飞行
		STATE_SITDOWN		= 0x00000020,   //是否坐下
		STATE_EXTEND_PROPERTY	= 0x00000040,   //是否有扩展状态
		STATE_ZOMBIE		= 0x00000080,	//是否尸体

		STATE_TEAM		= 0x00000100,   //是否队员
		STATE_TEAMLEADER	= 0x00000200,   //是否队长
		STATE_ADV_MODE		= 0x00000400,   //是否有广告内容
		STATE_MAFIA		= 0x00000800,   //是否帮派成员
		STATE_MARKET		= 0x00001000,	//是否正在摆摊
		STATE_FASHION_MODE	= 0x00002000,	//是否时装模式
		STATE_GAMEMASTER	= 0x00004000,	//后面存在着GM特殊状态
		STATE_PVPMODE		= 0x00008000,	//是否开启了PVP开关

		STATE_EFFECT		= 0x00010000,	//是否有特殊效果
		STATE_IN_PVP_COMBAT	= 0x00020000,	//是否正在PVP中
		STATE_IN_DUEL_MODE	= 0x00040000,	//是否正在决斗中
		STATE_MOUNT		= 0x00080000,	//正在骑乘中
		STATE_IN_BIND		= 0x00100000,	//和别人绑在一起
		STATE_BATTLE_OFFENSE	= 0x00200000,	//战争攻击方
		STATE_BATTLE_DEFENCE	= 0x00400000,	//战争防守方
		STATE_SPOUSE            = 0x00800000,   //存在配偶

		STATE_ELF_REFINE_ACTIVATE = 0x01000000, //当前装备的小精灵精炼效果激活 lgc
		STATE_SHIELD_USER		  = 0x02000000,	//完美神盾用户
		STATE_INVISIBLE			  = 0x04000000,	//进入隐身状态
		STATE_EQUIPDISABLED		  = 0x08000000,	//有装备已经失效
		STATE_FORBIDBESELECTED	  = 0x10000000,	//禁止被其他人选中
		STATE_PLAYERFORCE	  	  = 0x20000000,	//已加入势力
		STATE_MULTIOBJ_EFFECT	  = 0x40000000,	//与其他对象存在特殊效果
		STATE_COUNTRY			  = 0x80000000,	//已加入国家

		STATE_STATE_CORPSE	= 0x00000008,	//NPC是否出于尸体残留，这个和ZOMBIE不一样，非尸体残留的也可能ZOMBIE
		STATE_NPC_ADDON1	= 0x00000100,	//NPC附加属性位1 
		STATE_NPC_ADDON2	= 0x00000200,	//NPC附加属性位2 
		STATE_NPC_ADDON3	= 0x00000400,	//NPC附加属性位3 
		STATE_NPC_ADDON4	= 0x00000800,	//NPC附加属性位4 
		STATE_NPC_ALLADDON	= 0x00000F00,	//NPC附加属性位 
		STATE_NPC_PET		= 0x00001000,	//NPC是一个PET，后面跟随PET的主人ID
		STATE_NPC_NAME		= 0x00002000,	//NPC有独特的名字，后面跟随一字节char和名字内容
		STATE_NPC_FIXDIR	= 0x00004000,	//NPC朝向不变
		STATE_NPC_MAFIA		= 0x00008000,	//NPC帮派ID

		//这里的枚举量是放到object_state2上的
		STATE_KING				= 0x00000001,	//是国王
		STATE_TITLE				= 0x00000002,   //称号
		STATE_REINCARNATION		= 0x00000004,   //转生
		STATE_REALMLEVEL		= 0x00000008,   //境界等级
		STATE_IN_COMBAT			= 0x00000010,	//战斗状态
		STATE_MAFIA_PVP_MASK    = 0x00000020,   //帮派pvp 状态
};

inline bool IsValid(int id){ return id!=-1; }
inline bool IsPlayer(int id){ return (id&0x80000000) == 0; }
inline bool IsNPC(int id){ return (id&0xC0000000) == 0x80000000; }
inline bool IsMatter(int id){ return (id&0xC0000000) == 0xC0000000; }
struct object_info
{
	int id;
	A3DVECTOR pos;
	int tid;	//模板id, npc和matter有
	bool zombie;//player特有
	int hp;
	object_info():id(-1),pos(0.f,0.f,0.f),tid(0),zombie(false),hp(100){}
};

template<typename T>
void MakeObjectInfo(const T &, object_info &);
template<>
inline void MakeObjectInfo(const S2C::INFO::player_info_1 & p_info, object_info & info)
{
	info.id = p_info.cid;
	info.pos = p_info.pos;
	info.zombie = (p_info.object_state & STATE_ZOMBIE);
}
template<>
inline void MakeObjectInfo(const S2C::INFO::npc_info & n_info, object_info & info)
{
	info.id = n_info.nid;
	info.pos = n_info.pos;
	info.tid = n_info.tid;
}
template<>
inline void MakeObjectInfo(const S2C::INFO::matter_info_1 & m_info, object_info & info)
{
	info.id = m_info.mid;
	info.pos = m_info.pos;
	info.tid = m_info.tid;
}
		
struct item
{
	int type;
	size_t count;
	item():type(-1),count(0){}
};

struct skill
{
	int id;
	int level;
	int mp_cost;
	float range;
};

struct Player
{
	SpinLock lock;
	bool active;
	Player* next; 
	int id;
	PlayerImp * imp;
	
public:
	Player(){ Clear(); }
	void Clear()
	{
		active = false;
		next = NULL;
		id = 0;
		imp = NULL;
	}
	void Lock(){ lock.Lock(); }
	void Unlock(){ lock.UNLock(); }
};

class PlayerImp
{
	Player * parent;
	bool is_zombie;
	//basic info
	int level;
	int exp;
	//
	int hp;
	int max_hp;
	int mp;
	int max_mp;

	float attack_range;
	float run_speed;

	size_t money;
	size_t money_capacity;
	std::vector<item> equipment;
	std::vector<item> inventory;

	std::vector<skill> attack_skills;
	
	//move info
	A3DVECTOR pos;
	unsigned short move_seq;
	bool is_moving;
	unsigned char dir;
	//
	task_basic * cur_task;
	std::list<task_basic *> task_list;
	//
	int ai_mode;
	int selected_target;
	int follow_target;
	int action_failed;
	char force_attack;
	//
	std::map<int,object_info> object_list;
	
public:
	enum
	{
		MAX_PLAYER_TASK = 10,
	};
	enum
	{
		AI_MODE_IDLE = 0,
		AI_MODE_ZOMBIE,
		AI_MODE_DEBUG,
		AI_MODE_PATROL,
		AI_MODE_FOLLOW,
		AI_MODE_AUTO,
		AI_MODE_MAX,
	};
	enum
	{
		IL_INVENTORY,
		IL_EQUIPMENT,
		IL_TASK_INVENTORY,
	};
	PlayerImp(Player * _parent);
	~PlayerImp();
	void Init();
	void Release();
	void Tick();

public:
	bool AddTask(task_basic * task);
	bool StartTask();
	bool EndCurTask();
	void ClearTask();
	bool HasNextTask(){ return !task_list.empty();}

	void AITick();
	void SwitchAIMode(int mode);
	void IdleModeTick();
	void PatrolModeTick();
	void FollowModeTick();
	void AutoModeTick();
	void DebugModeTick();
	void ZombieModeTick();
	int GatherTarget(float range);
	bool QueryTargetInfo(int id, object_info& info);
	int DetermineAttackSkill(float & squared_range);
	bool CheckSkillCondition(skill& sk);

public:
	bool MatterCanPickup(int tid, size_t & pile_limit);
	bool MatterCanGather(int tid);
	bool NpcCanAttack(int tid);
	bool PlayerCanAttack(int id);
	bool IsAttackSkill(int id, int & mp_cost, float & range);

public:
	size_t GetItemCount(std::vector<item>& inv, int type);	
	int GetItemIndex(std::vector<item>& inv, int type);	
	//bool IsItemExist(std::vector<item>& inv, int type){ return GetItemIndex(inv, type) != -1; }
	bool IsItemExist(std::vector<item>& inv, int type, size_t index){ return index < inv.size() && inv[index].type == type; }
	int PushItem(int type, size_t & count, size_t pile_limit);
	int GetEmptySlot();

public:
	void IncMoveSeq(){ ++ move_seq; }
	void ResetMoveSeq(unsigned short seq){ move_seq = seq; }
	void MoveTo(A3DVECTOR & dest);
	void ResetPos(A3DVECTOR & p){ pos = p; }
	void MoveAgentLearn(const A3DVECTOR & pos);
	bool MoveAgentGetMovePos(const A3DVECTOR & cur_pos, const A3DVECTOR & dest_pos, float range, A3DVECTOR & next_pos);
	bool MoveAgentGetMovePos(const A3DVECTOR & cur_pos, int dir, float range, A3DVECTOR & next_pos);
	

public:
	void HandleS2CCmd(void * buf, size_t size);
	void HandleMessage(MSG * msg);

public:
	void SendMessage(MSG * msg);	//msg 必须是BuildMessage生成的

public:	
	//send C2S cmd
	void SendC2SGameData(void * buf, size_t size);
	void send_debug_cmd(short cmd, int param);
	void send_debug_cmd(short cmd, int param1, int param2);
	void send_debug_cmd(short cmd, void * buf, size_t size);
	void get_all_data();
	void player_move(A3DVECTOR & pos, int time, float speed, char mode);
	void player_stop_move(A3DVECTOR & pos, int time, float speed, char mode);
	void select_target(int id); 
	void unselect(); 
	void normal_attack(char force_attack);
	void cast_skill(int skill_id, char force_attack, size_t target_count, int* targets);
	void pickup(int id, int tid);
	void equip_item(unsigned char inv_index, unsigned char eq_index);
	void get_inventory_detail(unsigned char where);
	void resurrect_by_item();
	void resurrect_in_town();
	void check_security_passwd();
	void cancel_action();
	void logout();
};

#endif
