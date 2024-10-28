#ifndef __ONLINEGAME_GS_OBJECT_H__
#define __ONLINEGAME_GS_OBJECT_H__

#include <spinlock.h>
#include <common/types.h>
#include <common/packetwrapper.h>
#include <ASSERT.h>

#include "config.h"


class world;
class gobject_imp;
struct slice;
struct gobject
{
	enum 
	{
		MSG_MASK_PLAYER_MOVE  	= 0x0001,
//		MSG_MASK_NPC_MOVE	= 0x0002,
//		MSG_MASK_MATTER_DROP	= 0x0004,
//		MSG_MASK_PLAYER_DEATH	= 0x0008,
//		MSG_MASK_ACTIVE		= 0x0010,
		MSG_MASK_ATTACK		= 0x0020,
		MSG_MASK_ENCHANT	= 0x0040,
		MSG_MASK_CRY_FOR_HELP	= 0x0080,
	};

	int	spinlock;	//自旋锁
#ifdef _DEBUG
	int 	cur_msg;	//最近一条执行的消息号
#endif
	unsigned int crc;	//可能的crc校验值
	bool 	b_chk_state;	//表示该对象是否使用中的检查锁
	bool 	b_zombie;	//是否僵尸状态
	bool	b_disconnect;	//是否已经断线，只有player使用
	unsigned char dir;	//对象所处的方向
	int	msg_mask;	//接收消息的标志
	gobject * pPrev;	//前一个指针
	gobject * pNext;	//后一个指针
	slice 	* pPiece;	//所属的块指针
	A3DVECTOR pos;		//对象所处的位置
	XID 	ID;		//id,对于用户，使用uid,对于其他物品和NPC，使用 2位标志 | 14位 world_index | 16位索引
	gobject_imp * imp;	//实际的具体实现
	world * plane;
	float	body_size;	//对象的大小尺度 
	int 	collision_id;//碰撞标志位
public:
	inline void Lock() {mutex_spinlock(&spinlock);}
	inline void Unlock() {mutex_spinunlock(&spinlock);}
	inline int  TryLock() { return mutex_spinset(&spinlock);}
	inline bool IsEmpty() { return !b_chk_state;}
	inline bool IsActived() { return b_chk_state;}
	inline bool IsZombie() { return b_zombie;}
	inline void SetActive() { b_chk_state = true;}
	inline void ClrActive() { b_chk_state = false;}
	inline void Clear()
	{
		crc = 0;
		b_chk_state = false; 
		b_zombie = false;
		dir = 0;
		msg_mask = 0;
		ID.id = -1;
		ID.type = -1;
		imp = NULL;
		body_size = 0.f;
		b_disconnect = false;
		collision_id = 0;
	}
};

struct object_base_info
{
	int race;			//职业加性别 对于npc无效
	int faction;			//派系
	int level;			//级别
	int hp;				//hp
	int max_hp;			//mp
	int mp;				//法术 
};


struct gactive_object : public gobject
{
	object_base_info base_info;
	unsigned int object_state;	//这个表示对象的状态
	unsigned int object_state2;
	unsigned char shape_form;	//变身类型
	unsigned char emote_form;	//表情类型
	unsigned char effect_count;
	unsigned char npc_reborn_flag;	//对NPC而言，代表重生时需要重新生成
	unsigned int extend_state;
	unsigned int extend_state2;
	unsigned int extend_state3;
	unsigned int extend_state4;
	unsigned int extend_state5;
	unsigned int extend_state6;
	int	invisible_degree;		//当前人物或npc的隐身级别，未使用隐身技能时该值为0
	int anti_invisible_degree;	//当前人物或npc的反隐级别
	int multiobj_effect_count;
	struct
	{
		int target;
		char type;
	}multiobj_effect_list[MAX_MULTIOBJ_EFFECT_COUNT];

	template<typename WRAPPER>
	WRAPPER & Export(WRAPPER & wrapper)
	{
		wrapper << object_state << object_state2 << shape_form << effect_count << extend_state << extend_state2 << extend_state3 << extend_state4 << extend_state5 << extend_state6 << invisible_degree << anti_invisible_degree << multiobj_effect_count;
		return wrapper.push_back(multiobj_effect_list, sizeof(multiobj_effect_list));
	}

	template<typename WRAPPER>
	WRAPPER & Import(WRAPPER & wrapper)
	{
		wrapper >> object_state >> object_state2 >> shape_form >> effect_count >> extend_state >> extend_state2 >> extend_state3 >> extend_state4 >> extend_state5 >> extend_state6 >> invisible_degree >> anti_invisible_degree >> multiobj_effect_count;
		return wrapper.pop_back(multiobj_effect_list, sizeof(multiobj_effect_list));
	}

	void Clear()
	{
		extend_state = 0;
		extend_state2 = 0;
		extend_state3 = 0;
		extend_state4 = 0;
		extend_state5 = 0;
		extend_state6 = 0;
		object_state = 0;
		object_state2 = 0;
		shape_form = 0;
		emote_form = 0;//移动的时候不用存盘，因为此时不会有相应的表情
		effect_count = 0;
		memset(&base_info,0,sizeof(base_info));
		invisible_degree = 0;
		anti_invisible_degree = 0;
		multiobj_effect_count = 0;
		memset(multiobj_effect_list, 0, sizeof(multiobj_effect_list));
		gobject::Clear();
	}
	enum		
	{	
		//这里的枚举量是放到object_state上的
	//考虑可以将player专属内容放到和NPC专属内容平行的位置上
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
		STATE_NPC_FLY		= 0x00010000,	//NPC飞行
		STATE_NPC_SWIM		= 0x00020000,	//NPC游泳

		//这里的枚举量是放到object_state2上的
		STATE_KING				= 0x00000001,	//是国王
		STATE_TITLE				= 0x00000002,   //称号
		STATE_REINCARNATION		= 0x00000004,   //转生
		STATE_REALMLEVEL		= 0x00000008,   //境界等级
		STATE_IN_COMBAT			= 0x00000010,	//战斗状态
		STATE_MAFIA_PVP_MASK    = 0x00000020,   //帮派pvp 状态
		STATE_PLAYER_GENDER		= 0x00000040,	// gender
		STATE_MNFACTION_MASK	= 0x00000080,	//跨服唯一帮派id
		STATE_CASH_VIP_MASK     = 0x00000100    //VIP
	};

	inline bool IsFemale() const
	{
		return base_info.race < 0;
	}

	inline bool IsDuelMode() const
	{
		return object_state & STATE_IN_DUEL_MODE;
	}

	inline bool IsMountMode() const
	{
		return object_state & STATE_MOUNT;
	}

	inline bool IsFlyMode() const
	{
		return object_state & STATE_FLY;	
	}

	inline void SetBattleOffense() 
	{
		object_state &= ~STATE_BATTLE_DEFENCE;
		object_state |= STATE_BATTLE_OFFENSE;
	}

	inline void SetBattleDefence()
	{
		object_state &= ~STATE_BATTLE_OFFENSE;
		object_state |= STATE_BATTLE_DEFENCE;
	}

	inline void ClrBattleMode()
	{
		object_state &= ~(STATE_BATTLE_OFFENSE | STATE_BATTLE_DEFENCE);
	}

	inline bool IsBattleOffense() 
	{
		return object_state & STATE_BATTLE_OFFENSE;
	}

	inline bool IsBattleDefence()
	{
		return object_state & STATE_BATTLE_DEFENCE;
	}

	inline bool IsInvisible()
	{
		return object_state & STATE_INVISIBLE;	
	}

	inline bool IsKing()
	{
		return object_state2 & STATE_KING;
	}
};

struct gnpc: public gactive_object
{
	size_t spawn_index;
	int native_state;
	int tid;		//实际的tid
	int vis_tid;		//可见的tid
	int monster_faction;	//怪物的小类
	int cruise_timer;	//闲逛时的计数器
	int idle_timer;
	int idle_timer_count;	//用于在idle状态计时
	int npc_idle_heartbeat;	//如果在idle时使用何种方式记性加速记数
	int master_id;		//主人ID，只有处于PET状态的NPC才会有此状态
	int mafia_id;		//所属帮派id;
	short name_size;	//NPC的自动一名字长度，只有设置了名字标志的npc此状态才有效 
	char npc_name[18];	//NPC的自定义名字，只有设置了名字标志的npc此状态才有效
	enum
	{
		TYPE_NORMAL ,
		TYPE_NATIVE ,
		TYPE_EXTERN_NATIVE,
		TYPE_FREE,
	};
	inline gnpc* get_next() { return (gnpc*)pNext;}
	inline gnpc* get_prev() { return (gnpc*)pPrev;}
	void Clear()
	{
		tid = 0;
		vis_tid = 0;
		master_id = 0;
		mafia_id = 0;
		name_size = 0;
		native_state = TYPE_NORMAL;
		npc_idle_heartbeat = 0;
		gactive_object::Clear();
	}
	bool IsNative()
	{
		return native_state == TYPE_NATIVE;
	}

	template<typename WRAPPER>
	WRAPPER & Import(WRAPPER & wrapper)
	{
		gactive_object::Import(wrapper); 
		wrapper >> ID >> pos >> msg_mask >> tid >> vis_tid >> 
		base_info.race >> base_info.faction >>
		base_info.level >> base_info.hp >>
		base_info.max_hp >> monster_faction >> body_size;
		wrapper >> master_id >> mafia_id >> name_size;
		return wrapper.pop_back(npc_name,sizeof(npc_name));
	}
	
	template<typename WRAPPER>
	WRAPPER & Export(WRAPPER & wrapper)
	{
		gactive_object::Export(wrapper); 
		wrapper << ID << pos << msg_mask << tid << vis_tid << 
		base_info.race << base_info.faction <<
		base_info.level << base_info.hp <<
		base_info.max_hp << monster_faction << body_size;
		wrapper << master_id << mafia_id << name_size;
		return wrapper.push_back(npc_name,sizeof(npc_name));
	}
};

struct gmatter : public gobject
{
	unsigned char dir1;
	unsigned char rad;
	unsigned char matter_state;
	unsigned char matter_value;
	enum
	{
		STATE_MASK_NORMAL_MINE = 0x00,			//普通矿物
		STATE_MASK_DYN_OBJECT = 0x01,           //表示本物品为动态物品，其ID应从动态ID中寻找
		STATE_MASK_SOUL_MINE = 0x02,			//元魂
	};
	int matter_type;
	int spawn_index;		//只对矿产有用
	inline gmatter* get_next() { return (gmatter*)pNext;}
	inline gmatter* get_prev() { return (gmatter*)pPrev;}
	inline void SetDirUp(unsigned char d, unsigned char d1, unsigned char r)
	{
		dir = d;
		dir1 = d1;
		rad = r;
	}
	
	inline void SetMatterValue(unsigned char v)
	{
		matter_value = v;
	}

	inline void Clear()
	{
		gobject::Clear();
		dir1 = rad = matter_state = matter_value = 0;
	}

};
#endif

