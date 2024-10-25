#ifndef _ITEM_SKILLTRIGGER2_H_
#define _ITEM_SKILLTRIGGER2_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

//��Ҫʩ��Ŀ��ļ�����Ʒ

class skilltrigger2_item : public item_body
{
public:
	DECLARE_SUBSTANCE(skilltrigger2_item);
	
public:
	//item_body�д��麯��
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_SKILLTRIGGER2;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new skilltrigger2_item(*this);}
public:
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack);
	virtual bool IsItemCanUseWithTarget(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return 0;}//����ʹ��session_use_item_with_target,��֤skill_session��ʹ����Ʒ�����
	virtual bool IsItemBroadcastUse() {return true;}
	
	virtual bool GetSkillData(unsigned int& skill_id, unsigned int& skill_level);
};

#endif
