#ifndef __SKILL_MYOBJECT
#define __SKILL_MYOBJECT

#include "basicinfo.h"
#include "common/types.h"
#include "attack.h"
#include "property.h"

class gactive_imp 
{
public:
	gactive_imp(BasicInfo&, bool);
	bool attacker;
	int attack;
	int magic;
	extend_prop extend;
	basic_prop  basic;
	void HandleMessage(attack_msg& msg);
	void HandleMessage(enchant_msg& msg);
};



#endif

