#ifndef _ITEM_SKILLTRIGGER2_H_
#define _ITEM_SKILLTRIGGER2_H_

#include <stddef.h>
#include "../item.h" 
#include "../config.h"

//需要施放目标的技能物品

class skilltrigger2_item : public item_body
{
public:
	DECLARE_SUBSTANCE(skilltrigger2_item);
	
public:
	//item_body中纯虚函数
	ITEM_TYPE GetItemType()  { return ITEM_TYPE_SKILLTRIGGER2;}
	bool ArmorDecDurability(int) { return false;}
	item_body * Clone() const { return new skilltrigger2_item(*this);}
public:
	virtual int OnUseWithTarget(item::LOCATION l,int index,gactive_imp * obj,const XID & target, char force_attack);
	virtual bool IsItemCanUseWithTarget(item::LOCATION l) { return true;}
	virtual int OnGetUseDuration() { return 0;}//必须使用session_use_item_with_target,保证skill_session在使用物品后进行
	virtual bool IsItemBroadcastUse() {return true;}
	
	virtual bool GetSkillData(unsigned int& skill_id, unsigned int& skill_level);
};

#endif
