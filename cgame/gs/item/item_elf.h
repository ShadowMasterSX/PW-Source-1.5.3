/***************************************************************************
*
*	item_elf
*
***************************************************************************/
#ifndef _ITEM_ELF_H_
#define _ITEM_ELF_H_

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h" 
#include "../config.h"
#include <crc.h>
#include <vector.h>

#define MAX_ELF_EQUIP_CNT 	4
#define MAX_STAMINA			999999
#define INITIAL_VIGOR_GEN	1
#define INITIAL_MAX_VIGOR	100
#define INITIAL_SKILL_SLOT	3
#define MAX_ELF_SKILL_CNT 8
#define ATTRIBUTE_UPPER_LIMIT	40
#define GENIUS_UPPER_LIMIT	8
#define ELF_DECOMPOSE_EXP_LOSS 0.1
#define MAX_ELF_REFINE_LEVEL 36
#define NEED_ENERGY_PER_SKILL_SLOT 40

class gactive_imp;

/**************************************************************************
数据库中存储小精灵item_body的格式:(itemdataman.h中定义)
	struct _elf_item_content
	{
		struct _elf_essence ess;	//小精灵本体
		int equip_cnt;				//已装备的装备数量
		//unsigned int equipid[equip_cnt];		//装备id
		int skill_cnt;				//已学会的技能数
		//struct _elf_skill_data elfskill[skill_cnt];	//技能 id和等级
	};
***************************************************************************/
//精炼激活效果及每秒消耗标准值  最终每秒消耗耐力值=标准值*(角色当前等级+105)/210
//激活的一瞬间消耗耐力值=每秒消耗值*60。随后根据小精灵自身的等级和精炼的等级，每秒都消耗。
struct refine_effect
{
	short max_hp;
	short attack_degree;
	short defend_degree;
	short std_cost;
};
extern refine_effect elf_refine_effect_table[MAX_ELF_REFINE_LEVEL+1];
extern float elf_refine_succ_prob_ticket0[MAX_ELF_REFINE_LEVEL+1];
extern float elf_refine_succ_prob_ticket1[MAX_ELF_REFINE_LEVEL+1];
extern float elf_refine_succ_prob_ticket2[MAX_ELF_REFINE_LEVEL+1];
extern int elf_refine_max_use_ticket3[MAX_ELF_REFINE_LEVEL+1];
extern int elf_refine_transmit_cost[MAX_ELF_REFINE_LEVEL+1];
extern int elf_exp_loss_constant[MAX_PLAYER_LEVEL+1];

/***************************************************************************/
#pragma pack(1)
struct elf_skill_data		//保存在数据库中技能数据
{
	unsigned short id;
	short level;	
};
#pragma pack()

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const struct elf_skill_data & sk)
{
	wrapper.push_back(&sk, sizeof(sk));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, struct elf_skill_data & sk)
{
	wrapper.pop_back(&sk, sizeof(sk));
	return wrapper;
}

/***************************************************************************/

#pragma pack(1)
struct elf_essence			//定义需要保存在数据库中的属性 
{
	unsigned int exp;
	short level;
	
	short total_attribute;	//升级产生的属性点总数，不包括装备增加的及各属性初始值
	short strength;			//由加属性点而产生的属性值，不包括装备增加的及各属性初始值
	short agility;
	short vitality;
	short energy;

	short total_genius;		//天赋点，不包括装备增加的
	short genius[5];			//金木水火土0-4
	
	short refine_level;
	int stamina; 			//耐力
	int status_value;		//0: 安全状态，g_timer.get_systime():转化状态，-1:可交易状态 
};
#pragma pack()

template <typename WRAPPER>
WRAPPER & operator<<(WRAPPER & wrapper, const elf_essence & es)
{
	wrapper.push_back(&es, sizeof(es));
	return wrapper;
}

template <typename WRAPPER>
WRAPPER & operator>>(WRAPPER & wrapper, elf_essence & es)
{
	wrapper.pop_back(&es, sizeof(es));
	return wrapper;
}


struct elf_extend_prop
{
	//以下可从模板中获取或由其他属性生成，不需保存至数据库
	float exp_factor;
	int max_rand_prop;		//升级时随机获取属性点总数的最大值
	int average_rand_prop;	//升级时随机获取属性点总数的平均值
	
	short cur_strength;			//包括装备增加的及各属性初始值
	short cur_agility;
	short cur_vitality;
	short cur_energy;

	short cur_genius[5];			//包括装备增加的，金木水火土0-4
	
	unsigned int all_equip_mask; //当前都装备了什么类型装备 bit 1- equip 0-none，应该只使用bit 0--3
	int cur_skill_slot;		//当前可学技能最大值
	int secure_status;			//0 安全状态  elf_item::enum{}
};

/***************************************************************************/

class elf_item : public item_body
{
public:
	typedef	abase::vector<struct elf_skill_data, abase::fast_alloc<> > SKILL_VECT;		
	typedef abase::vector<unsigned int, abase::fast_alloc<> > EQUIP_VECT;
	enum {				//安全状态，转化状态，可交易状态
		STATUS_SECURE = 0,
		STATUS_TRANSFORM,
		STATUS_TRADABLE,		
	};
private:
	struct elf_essence ess;
	EQUIP_VECT equipvect;
	SKILL_VECT skillvect;		
	struct elf_extend_prop prop;
	int stamina_offset;
	
public:
	DECLARE_SUBSTANCE(elf_item);
	//存储相关
	bool SaveEssence(archive & ar);
	bool LoadEssence(archive & ar);
	bool SaveEquip(archive & ar);
	bool LoadEquip(archive & ar);
	bool SaveSkill(archive & ar);
	bool LoadSkill(archive & ar);
	bool Save(archive & ar);
	bool Load(archive & ar);

	bool UpdateEssenceData();
	bool UpdateEquipData();
	bool UpdateSkillData();
	void ClearData();	
	void OnRefreshItem();	//更新生成属性elf_extend_prop,ess.stamina,_raw_data
	void OnRefreshRawData();	//仅更新ess.stamina,_raw_data
	
public:
	//Get prop
	int GetStatusValue(){return ess.status_value;}
	int GetSecureStatus(){return prop.secure_status;}
	int OnGetLevel(){return ess.level;}//item_body中接口GetLevel()
	int GetStamina(){return ess.stamina + stamina_offset;}
	short GetRefineLevel(){return ess.refine_level;}
	bool IsElfItemExist(int mask){return prop.all_equip_mask & mask;}
public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_ELF;}
	bool ArmorDecDurability(int) { return false;}
	elf_item * Clone() const{	return new elf_item(*this);	}	

public:
	//小精灵操作
	//通过人物对已装备的小精灵操作
	bool AddAttributePoint(short str, short agi, short vit, short eng, bool ischeck);//加属性点，单个属性超过40有损耗
	bool AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck);//加天赋点
	unsigned int InsertExp(unsigned int exp, short exp_level, gactive_imp* imp, bool& is_levelup, bool ischeck);//注入经验，参数：想要注入的经验数，返回：实际注入的经验数
	bool EquipElfItem(unsigned int id, bool ischeck);//装备小精灵的装备
	bool ChangeElfSecureStatus(int status, bool ischeck);//安全状态切换
	void UpdateElfSecureStatus();//用于转化状态->可交易状态的自动转化
	int OnCharge(int element_level, size_t count, int & cur_time);	//加耐力，使用飞剑接口
	void DecStamina(int sta){ stamina_offset -= sta;}//减耐力，技能施放、精炼效果激活消耗
	
	//通过npc服务，未装备小精灵
	bool DecAttributePoint(short str, short agi, short vit, short eng);//洗属性点
	bool FlushGeniusPoint();//全洗天赋点
	int LearnSkill(gactive_imp * imp, unsigned short skill_id);//函数中消耗玩家元神金钱(技能书), >0 newlevel -1 error
	int ForgetSkill(gactive_imp * imp, unsigned short skill_id, short forget_level);//>0 newlevel -1 error
	int ElfRefine(int ticket_id, int ticket_cnt, int& original_level);
	short SetRefineLevel(short level);//精炼等级转移时调用
	int DestroyElfItem(int mask,int equip_type);//mask 装备位置,type -1则只销毁 >0将装备替换,成功:返回原有装备id，失败返回-1
private:
	//小精灵操作中使用的私有函数
	double GetExpObtainFactor(short exp_level, short elf_level);
	bool LevelUp(gactive_imp* imp);//小精灵升级，属性点、天赋点增加
	bool GetDecomposeElfExp(unsigned int & exp, int & exp_level);//返回得到的小精灵经验丸的经验值
	void CheckActiveSkill(struct elf_skill_data skilldata[], int & skillcnt, gactive_imp* imp);//仅在OnActivate中调用
	bool CheckRawExp()
	{
		return ((elf_essence*)(_raw_data.begin()))->exp == ess.exp;	
	}
	void UpdateRawExp()
	{
		((elf_essence*)(_raw_data.begin()))->exp = ess.exp;
	}
public:
	//装备小精灵相关
	bool VerifyRequirement(item_list & list, gactive_imp* imp);
	void OnActivate(item::LOCATION, size_t pos, size_t count, gactive_imp* imp);
	void OnDeactivate(item::LOCATION, size_t pos, size_t count,gactive_imp* imp);
	virtual void OnPutIn(item::LOCATION l,item_list & list,size_t pos,size_t count,gactive_imp* obj); 
	virtual void OnTakeOut(item::LOCATION l,size_t pos, size_t count, gactive_imp* obj);
	bool IsItemCanUse(item::LOCATION l){return l == item::BODY && ess.refine_level >= 1 && ess.refine_level <= MAX_ELF_REFINE_LEVEL; }
	bool IsItemSitDownCanUse(item::LOCATION l){return l == item::BODY && ess.refine_level >= 1 && ess.refine_level <= MAX_ELF_REFINE_LEVEL;}
	int OnUse(item::LOCATION l,gactive_imp * imp,size_t count);

public:
	abase::octets _raw_data;
	unsigned short _crc;
	unsigned short GetDataCRC() { return _crc; }
	void CalcCRC()
	{   
		ASSERT(_raw_data.size() > 0);
		_crc = crc16( (unsigned char *)_raw_data.begin(), _raw_data.size());
	}
	void GetItemData(const void ** data, size_t &len)
	{
		if(stamina_offset != 0)
			OnRefreshRawData();
		
		*data = _raw_data.begin();
		len = _raw_data.size();
 	}
	int GetIdModify()
	{
		int mask  = ess.refine_level & 0xFF;
		mask <<= 24;
		int cur_rand_prop = ess.total_attribute - ess.level + 1;
		if(cur_rand_prop <= prop.average_rand_prop*0.8f)
			mask |= (1 << 16);
		else if(cur_rand_prop <= prop.average_rand_prop)
			mask |= (2 << 16);
		else if(cur_rand_prop <= prop.average_rand_prop*0.8f + prop.max_rand_prop*0.2f)
			mask |= (3 << 16);
		else
			mask |= (4 << 16);
		return mask;
	}
	
public:
	
	elf_item()
	{
		memset(&ess, 0, sizeof(ess));
		memset(&prop, 0, sizeof(prop));
		stamina_offset = 0;
		_crc = 0;
	}

//for debug only
	void dump_all();
	void change_elf_property(int index, int value, gactive_imp* imp);
	void dump_skill(char * buf, size_t buf_size);
};

#endif
