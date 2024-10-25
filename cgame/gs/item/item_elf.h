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
���ݿ��д洢С����item_body�ĸ�ʽ:(itemdataman.h�ж���)
	struct _elf_item_content
	{
		struct _elf_essence ess;	//С���鱾��
		int equip_cnt;				//��װ����װ������
		//unsigned int equipid[equip_cnt];		//װ��id
		int skill_cnt;				//��ѧ��ļ�����
		//struct _elf_skill_data elfskill[skill_cnt];	//���� id�͵ȼ�
	};
***************************************************************************/
//��������Ч����ÿ�����ı�׼ֵ  ����ÿ����������ֵ=��׼ֵ*(��ɫ��ǰ�ȼ�+105)/210
//�����һ˲����������ֵ=ÿ������ֵ*60��������С��������ĵȼ��;����ĵȼ���ÿ�붼���ġ�
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
struct elf_skill_data		//���������ݿ��м�������
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
struct elf_essence			//������Ҫ���������ݿ��е����� 
{
	unsigned int exp;
	short level;
	
	short total_attribute;	//�������������Ե�������������װ�����ӵļ������Գ�ʼֵ
	short strength;			//�ɼ����Ե������������ֵ��������װ�����ӵļ������Գ�ʼֵ
	short agility;
	short vitality;
	short energy;

	short total_genius;		//�츳�㣬������װ�����ӵ�
	short genius[5];			//��ľˮ����0-4
	
	short refine_level;
	int stamina; 			//����
	int status_value;		//0: ��ȫ״̬��g_timer.get_systime():ת��״̬��-1:�ɽ���״̬ 
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
	//���¿ɴ�ģ���л�ȡ���������������ɣ����豣�������ݿ�
	float exp_factor;
	int max_rand_prop;		//����ʱ�����ȡ���Ե����������ֵ
	int average_rand_prop;	//����ʱ�����ȡ���Ե�������ƽ��ֵ
	
	short cur_strength;			//����װ�����ӵļ������Գ�ʼֵ
	short cur_agility;
	short cur_vitality;
	short cur_energy;

	short cur_genius[5];			//����װ�����ӵģ���ľˮ����0-4
	
	unsigned int all_equip_mask; //��ǰ��װ����ʲô����װ�� bit 1- equip 0-none��Ӧ��ֻʹ��bit 0--3
	int cur_skill_slot;		//��ǰ��ѧ�������ֵ
	int secure_status;			//0 ��ȫ״̬  elf_item::enum{}
};

/***************************************************************************/

class elf_item : public item_body
{
public:
	typedef	abase::vector<struct elf_skill_data, abase::fast_alloc<> > SKILL_VECT;		
	typedef abase::vector<unsigned int, abase::fast_alloc<> > EQUIP_VECT;
	enum {				//��ȫ״̬��ת��״̬���ɽ���״̬
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
	//�洢���
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
	void OnRefreshItem();	//������������elf_extend_prop,ess.stamina,_raw_data
	void OnRefreshRawData();	//������ess.stamina,_raw_data
	
public:
	//Get prop
	int GetStatusValue(){return ess.status_value;}
	int GetSecureStatus(){return prop.secure_status;}
	int OnGetLevel(){return ess.level;}//item_body�нӿ�GetLevel()
	int GetStamina(){return ess.stamina + stamina_offset;}
	short GetRefineLevel(){return ess.refine_level;}
	bool IsElfItemExist(int mask){return prop.all_equip_mask & mask;}
public:
	//item_body�д��麯��
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_ELF;}
	bool ArmorDecDurability(int) { return false;}
	elf_item * Clone() const{	return new elf_item(*this);	}	

public:
	//С�������
	//ͨ���������װ����С�������
	bool AddAttributePoint(short str, short agi, short vit, short eng, bool ischeck);//�����Ե㣬�������Գ���40�����
	bool AddGeniusPoint(short g0, short g1, short g2, short g3, short g4, bool ischeck);//���츳��
	unsigned int InsertExp(unsigned int exp, short exp_level, gactive_imp* imp, bool& is_levelup, bool ischeck);//ע�뾭�飬��������Ҫע��ľ����������أ�ʵ��ע��ľ�����
	bool EquipElfItem(unsigned int id, bool ischeck);//װ��С�����װ��
	bool ChangeElfSecureStatus(int status, bool ischeck);//��ȫ״̬�л�
	void UpdateElfSecureStatus();//����ת��״̬->�ɽ���״̬���Զ�ת��
	int OnCharge(int element_level, size_t count, int & cur_time);	//��������ʹ�÷ɽ��ӿ�
	void DecStamina(int sta){ stamina_offset -= sta;}//������������ʩ�š�����Ч����������
	
	//ͨ��npc����δװ��С����
	bool DecAttributePoint(short str, short agi, short vit, short eng);//ϴ���Ե�
	bool FlushGeniusPoint();//ȫϴ�츳��
	int LearnSkill(gactive_imp * imp, unsigned short skill_id);//�������������Ԫ���Ǯ(������), >0 newlevel -1 error
	int ForgetSkill(gactive_imp * imp, unsigned short skill_id, short forget_level);//>0 newlevel -1 error
	int ElfRefine(int ticket_id, int ticket_cnt, int& original_level);
	short SetRefineLevel(short level);//�����ȼ�ת��ʱ����
	int DestroyElfItem(int mask,int equip_type);//mask װ��λ��,type -1��ֻ���� >0��װ���滻,�ɹ�:����ԭ��װ��id��ʧ�ܷ���-1
private:
	//С���������ʹ�õ�˽�к���
	double GetExpObtainFactor(short exp_level, short elf_level);
	bool LevelUp(gactive_imp* imp);//С�������������Ե㡢�츳������
	bool GetDecomposeElfExp(unsigned int & exp, int & exp_level);//���صõ���С���龭����ľ���ֵ
	void CheckActiveSkill(struct elf_skill_data skilldata[], int & skillcnt, gactive_imp* imp);//����OnActivate�е���
	bool CheckRawExp()
	{
		return ((elf_essence*)(_raw_data.begin()))->exp == ess.exp;	
	}
	void UpdateRawExp()
	{
		((elf_essence*)(_raw_data.begin()))->exp = ess.exp;
	}
public:
	//װ��С�������
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
