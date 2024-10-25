#ifndef __ONLINEGAME_GS_PETEGG_ITEM_H__
#define __ONLINEGAME_GS_PETEGG_ITEM_H__

#include <stddef.h>
#include <octets.h>
#include <common/packetwrapper.h>
#include "../item.h"
#include "../config.h"
#include <crc.h>

#pragma pack(1)
struct pe_essence
{
	int require_level;	//������Ҽ���
	int require_class;	//�������ְҵ
	int honor_point;        //�øж�
	int pet_tid;            //�����ģ��ID
	int pet_vis_tid;        //����Ŀɼ�ID�����Ϊ0�����ʾ������ɼ�ID��
	int pet_egg_tid;        //���ﵰ��ID
	int pet_class;          //�������� ս�裬��裬���ͳ�
	short level;            //���Ｖ��
	unsigned short color;   //������ɫ�����λΪ1��ʾ��Ч��Ŀǰ���������Ч
	int exp;                //���ﵱǰ����
	int skill_point;        //ʣ�༼�ܵ�
	unsigned short name_len;//���ֳ��� 
	unsigned short skill_count;//��������
	char name[16];          //��������
	struct
	{
		int skill;
		int level;
	}skills[];
};

struct evo_prop
{
	int r_attack;
	int r_defense;
	int r_hp;
	int r_atk_lvl;
	int r_def_lvl;
	int nature;
}; //�������ר�����ԣ����ϵ�����Ը�
#pragma pack()

class item_pet_egg: public item_body
{
	pe_essence * _ess;
	size_t _size;
protected:
	virtual bool IsItemCanUse(item::LOCATION l) { return false;}
	virtual bool ArmorDecDurability(int) { return false;}
	virtual int OnGetUseDuration() { return 60;}	
	virtual bool IsItemBroadcastUse() {return true;}
	virtual bool VerifyRequirement(item_list & list,gactive_imp* obj) { return false;}
	virtual int OnUse(item::LOCATION l,int index,gactive_imp * obj,size_t count);

public:
	item_pet_egg():_ess(NULL),_size(0)
	{}

	item_pet_egg(const item_pet_egg & rhs)
	{
		_size = rhs._size;
		if(_size)
		{
			_ess = (pe_essence*)abase::fastalloc(_size);
			memcpy(_ess,rhs._ess,_size);
		}

	}
	

	~item_pet_egg()
	{
		if(_ess)
		{
			abase::fastfree(_ess,_size);
		}
	}
	

public:
	virtual ITEM_TYPE GetItemType()
	{
		return ITEM_TYPE_PET_EGG;
	}

	virtual void GetItemData(const void ** data, size_t &len)
	{
		*data = _ess; 
		len = _size;
	}

	virtual item_body* Clone() const
	{ 
		return  new item_pet_egg(*this); 
	}

	virtual bool Save(archive & ar)
	{
		ar.push_back(_ess,_size);
		return true;
	}

	virtual bool Load(archive & ar);

public:
	DECLARE_SUBSTANCE(item_pet_egg);

};
#endif

