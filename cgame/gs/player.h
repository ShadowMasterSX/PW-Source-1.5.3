#ifndef __ONLINEGAME_GS_PLAYER_H__
#define __ONLINEGAME_GS_PLAYER_H__

#include <time.h>
#include "object.h"
#include <common/packetwrapper.h>

struct gplayer  : public gactive_object
{
	//base_info.race
	//player的职业/种族/性别 最高位表示性别，低字节表示职业（目前没有种族设定）
	enum{
		EMPTY 		= 0,
		WAITING_LOGIN 	= 1,
		WAITING_ENTER	= 2,
		LOGIN_OK	= 3,	//登陆的正常状态
		WAITING_LOGOUT	= 4,
		DISCONNECTED 	= 5,
		WAITING_SWITCH  = 6,
	};
	inline gplayer * get_next() { return (gplayer*)pNext;}
	inline gplayer * get_prev() { return (gplayer*)pPrev;}
	int	cs_index;		//连接服务器索引
	int 	cs_sid;			//连接服务器session ID
	int	login_state;		//当前登录状态，在未生成imp时使用
	int 	id_mafia;		//帮派id
	short	effect_list[MAX_PLAYER_EFFECT_COUNT];		//这个标记的数目在gactive_object里声明了
	unsigned char rank_mafia;	//帮派内角色
	unsigned char market_id;	//当前摊号
	unsigned char vehicle;		//旅行的时的交通工具
	unsigned char gm_invisible;	//是否GM隐身,隐身后拥有完全视野
	unsigned short  custom_crc;	//当前的个性化数据crc
	int 	adv_data1;
	int 	adv_data2;
	int 	pariah_state;		//红名状态
	char	sec_level;		//修真级别
	char 	bind_type;		//绑定类型 1下面 2 上面 
	unsigned short mount_color;		//骑的马的颜色
	int	bind_target;		//绑定的人 是谁
	int 	mount_id;		//骑的马是什么类型
	int 	spouse_id;
	int 	team_id;	//所在队伍id(队长id),应该与player_team中的leader保持一致,用于隐身状态下广播消息是否发送的判断
	int64_t	disabled_equip_mask;	//失效的装备mask,如果相应位被置位,则相应的装备无论是否满足激活条件均不激活
	int		force_id;	//玩家势力id
	int		country_id;	//国家id
	unsigned short title_id;	// 当前称号，应该与player_title中的curr_title_id 保持一致
	unsigned char reincarnation_times;	//转生次数
	unsigned char realmlevel;	// 境界等级	 	
	unsigned char mafia_pvp_mask; // 帮派pvp Mask

	bool	is_waitting_login() { return login_state == WAITING_LOGIN;}
	void Clear()
	{
		cs_sid = -1;
		cs_index = -1;
		login_state =  EMPTY;
		id_mafia = 0;
		rank_mafia = 0;
		market_id = 0;
		ID.id = 0;
		base_info.race = -1;
		custom_crc = 0;
		vehicle = gm_invisible = 0;
		adv_data1 = adv_data2 = 0;
		bind_type  =0;
		sec_level = 0;
		mount_color = 0;
		bind_target = 0;
		mount_id = 0;
		spouse_id = 0;
		team_id = 0;
		disabled_equip_mask = 0;
		force_id = 0;
		country_id = 0;
		title_id = 0;
		reincarnation_times = 0;
		realmlevel = 0;
		mafia_pvp_mask = 0;
		gactive_object::Clear();
	}

	template <typename WRAPPER>
	WRAPPER &Export(WRAPPER & wrapper)
	{
		gactive_object::Export(wrapper);
		wrapper << ID << pos << msg_mask << crc << 
		id_mafia << rank_mafia << market_id << 
		vehicle << gm_invisible << custom_crc <<
		base_info.race << base_info.faction <<
		base_info.level << base_info.hp <<
		base_info.max_hp << body_size << 
		cs_sid << cs_index << login_state << 
		adv_data1 << adv_data2 << pariah_state << sec_level << 
		bind_type << mount_color <<
		bind_target << mount_id << spouse_id << team_id << disabled_equip_mask << 
		force_id << country_id << title_id << reincarnation_times << realmlevel << mafia_pvp_mask;
		return wrapper.push_back(effect_list,sizeof(effect_list));
	}

	template <typename WRAPPER>
	WRAPPER & Import(WRAPPER & wrapper)
	{
		gactive_object::Import(wrapper);
		wrapper >> ID >> pos >> msg_mask >> crc >> 
		id_mafia >> rank_mafia >> market_id >>
		vehicle >> gm_invisible >> custom_crc >> 
		base_info.race >> base_info.faction >>
		base_info.level >> base_info.hp >>
		base_info.max_hp >> body_size >>
		cs_sid >> cs_index >> login_state >> 
		adv_data1 >> adv_data2 >> pariah_state >> sec_level >>
		bind_type >> mount_color >>
		bind_target >> mount_id >> spouse_id >> team_id >> disabled_equip_mask >> 
		force_id >> country_id >> title_id >> reincarnation_times >> realmlevel >> mafia_pvp_mask;
		return wrapper.pop_back(effect_list,sizeof(effect_list));
	}
public:
	
};

inline bool make_link_sid(gplayer * dest, link_sid & id)
{
	id.cs_id = dest->cs_index;
	id.cs_sid = dest->cs_sid;
	id.user_id = dest->ID.id;
	return id.cs_id >= 0;
}

//副本或者其他世界的key，在进入副本世界时需要此种key
struct  instance_key
{
	struct key_essence
	{
		int key_level1;			//一级key， 有些副本需要这个 标准key是自己的ID
		struct 
		{
			int first;
			int second;
		}key_level2;			//二级key，有些副本需要这个  标准key是自己的队伍ID和seq
		int key_level3;			//三级key，有些副本需要这个  标准key是自己的帮派ID
		int key_level4;			//四级key，有些副本需要这个，这是为专有副本准备的（比如城战）
		int key_level5;			//五级key，有些副本需要这个，标准key是自己的国家ID
	}; 
	
	key_essence essence;
	key_essence target;
	int control_id;				//区域的控制号，当开启副本时，某些副本会根据这个id来生成某些怪物0表示非开启
	int special_mask;			//0x01 - GM  IK_SPECIAL_MASKG
};

enum IK_SPECIAL_MASK
{
	IKSM_GM 		= 0x1,
	IKSM_REENTER 	= 0x2,
};

template <typename WRAPPER >
WRAPPER & operator << (WRAPPER & wrapper ,const instance_key & rhs)
{
	return wrapper.push_back(&rhs, sizeof(rhs));
}

template <typename WRAPPER >
WRAPPER & operator >>  (WRAPPER & wrapper ,instance_key & rhs)
{
	return wrapper.pop_back(&rhs, sizeof(rhs));
}

struct instance_hash_key
{
	int key1;
	int key2;
	instance_hash_key(int k1 = 0, int k2 = 0) : key1(k1), key2(k2){}
	bool operator ==(const instance_hash_key & rhs) const
	{
		return key1 == rhs.key1 && key2 == rhs.key2;
	}
};

struct instance_hash_function
{
	inline long operator()(const instance_hash_key & key) const
	{
		return key.key1 ^ key.key2;
	}
};

struct player_var_data
{
	enum 
	{
		BASIC_VAR_DATA_SIZE = 16,
		VERSION1	= 1,
		VERSION2	= 2,
		VERSION3	= 3,
		VERSION4	= 4,
		VERSION5	= 5,
		VERSION6	= 6,
		VERSION7	= 7,
		CUR_VERSION
	};
	int version;
	int pk_count;		//杀人数目 
	int pvp_cooldown;	//pvp冷却时间
	bool pvp_flag;		//是否激活了pvp
	char dead_flag;		//是否死亡状态(3种)
	bool is_drop;		//是否在空中
	bool resurrect_state;	//复活状态(是否可原地复活)
	float resurrect_exp_reduce; //复活状态时损失的exp（可原地复活后才有效)
	instance_hash_key ins_key;  //上次退出时所在的副本hash key
	int  trashbox_size;	    //仓库的大小
	int last_instance_timestamp;//最后一次进入副本的时间戳
	int last_instance_tag;	    //最后一次进入副本的ID
	A3DVECTOR last_instance_pos;//最后一次进入副本的坐标
	int dir;		    //玩家下线时的方向
	float 	resurrect_hp_factor;		//复活状态后回血比例（可原地复活后才有效)
	float 	resurrect_mp_factor;		//复活状态时回蓝比例（可原地复活后才有效)
	int instance_reenter_timeout;		// 表示副本能重新进入的时间点  0表示不能进入
	int 		last_world_type;		// 最后一次退出时的世界类型
	A3DVECTOR	last_logout_pos;		// 最后一次退出时的坐标位置 (非存盘位置)

	template <typename PLAYER_IMP>
	void MakeData(gplayer * pPlayer, PLAYER_IMP * pImp)
	{
		version = CUR_VERSION;
		pk_count = pImp->GetPKCount();
		pvp_cooldown = pImp->GetPVPCoolDown();
		pvp_flag = pImp->GetPVPFlag();
		dead_flag =  pImp->GetDeadFlag();
		is_drop =  pImp->_layer_ctrl.IsOnAir();
		resurrect_state = pImp->GetResurrectState(resurrect_exp_reduce,resurrect_hp_factor,resurrect_mp_factor);
	//	int tag = world_manager::GetWorldTag();
		ins_key = pImp->_plane->w_ins_key;
		trashbox_size = pImp->_trashbox.GetTrashBoxSize();
		pImp->GetLastInstancePos( last_instance_tag, last_instance_pos,last_instance_timestamp);
		dir = pPlayer->dir;
		
		instance_reenter_timeout = pImp->GetInstanceReenterTimeout();
		last_world_type = pImp->GetWorldType();
		last_logout_pos = pPlayer->pos;
	}

	template <typename PLAYER_IMP>
	static void SetData(gplayer * pPlayer, PLAYER_IMP * pImp, const void * buf, size_t size)
	{
		player_var_data * pVar = (player_var_data*)buf;
		switch(pVar->version)
		{
			case VERSION1:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
				}
				break;
			case VERSION2:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
					pImp->SetResurrectState(pVar->resurrect_state,pVar->resurrect_exp_reduce,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR);
				}
				break;
			case VERSION3:
			case VERSION4:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
					pImp->SetResurrectState(pVar->resurrect_state,pVar->resurrect_exp_reduce,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR);
				}
				break;
			case VERSION5:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
					pImp->SetResurrectState(pVar->resurrect_state,pVar->resurrect_exp_reduce,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR);
				}
				break;
			case VERSION6:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
					pImp->SetResurrectState(pVar->resurrect_state,pVar->resurrect_exp_reduce,DEFAULT_RESURRECT_HP_FACTOR,DEFAULT_RESURRECT_MP_FACTOR);
					pPlayer->dir = pVar->dir;
				}
				break;
			case VERSION7:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
					pImp->SetResurrectState(pVar->resurrect_state,pVar->resurrect_exp_reduce,pVar->resurrect_hp_factor,pVar->resurrect_mp_factor);
					pPlayer->dir = pVar->dir;
				}
				break;
			case CUR_VERSION:
				{
					pImp->SetPVPState(pVar->pk_count,pVar->pvp_cooldown,pVar->pvp_flag);
					pImp->SetDeadFlag(pVar->dead_flag);
					if(pVar->is_drop) pImp->_layer_ctrl.Landing();
					pImp->SetResurrectState(pVar->resurrect_state,pVar->resurrect_exp_reduce,pVar->resurrect_hp_factor,pVar->resurrect_mp_factor);
					pPlayer->dir = pVar->dir;
					pImp->InitInstanceReenter(pVar->last_instance_tag,pVar->last_world_type,pVar->ins_key,pVar->instance_reenter_timeout,pVar->last_logout_pos);
				}
				break;
		}
	}

	static int GetTrashBoxSize(const void * buf, size_t size)
	{
		if(size < sizeof(int))
		{
			return TRASHBOX_BASE_SIZE;
		}
		player_var_data * pVar = (player_var_data*)buf;
		switch(pVar->version)
		{
			case VERSION5:
			case VERSION6:
			case VERSION7:
			case CUR_VERSION:
				return pVar->trashbox_size;
			default:
			return TRASHBOX_BASE_SIZE;
		}
	}

	template <typename INSTANCE_KEY>
	static void GetInstanceKey(const void * buf, size_t size,INSTANCE_KEY & key)
	{
		memset(&key,0,sizeof(key));
		if(size < sizeof(int)) 
		{
			key.key1 = 0;
			key.key2 = time(NULL);
			return;
		}
		player_var_data * pVar = (player_var_data*)buf;
		switch(pVar->version)
		{
			case VERSION3:
			case VERSION4:
			case VERSION5:
			case VERSION6:
			case VERSION7:
			case CUR_VERSION:
				{
					key = pVar->ins_key;
				}
				return ;
		}
		key.key1 = 0;
		key.key2 = time(NULL);
		return;
	}

	template <int >
	static void GetLastInstance(const void * buf, size_t size,int &last_ins_tag, A3DVECTOR & pos,int & create_ts)
	{
		if(size < sizeof(int)) 
		{
			last_ins_tag = -1;
			create_ts = -1;
			return;
		}
		player_var_data * pVar = (player_var_data*)buf;
		switch(pVar->version)
		{
			case VERSION5:
			case VERSION6:
			case VERSION7:
			case CUR_VERSION:
				{
					last_ins_tag = pVar->last_instance_tag;
					pos = pVar->last_instance_pos;
					create_ts = pVar->last_instance_timestamp;
				}
				return ;
		}
		last_ins_tag = -1;
		create_ts = -1;
		return;
	}

	enum 
	{
		ALIVE 	= 0,
		KILLED_BY_PLAYER = 1,
		KILLED_BY_NPC = 2,
	};

};
enum
{
	PLAYER_MONSTER_TYPE = 256
};

#endif

