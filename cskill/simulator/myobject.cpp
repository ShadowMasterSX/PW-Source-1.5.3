#include "myobject.h"
#include "mywrapper.h"
#include "obj_interface.h"

extern SkillInfo ginfo;
extern BasicInfo gvictim;
extern MyWrapper gwrapper;

gactive_imp::gactive_imp(BasicInfo& info, bool b)
{
	attacker = b;
	basic.hp = info.hp;
	basic.mp = info.mp;
	basic.level = info.level;
	attack = info.attack;
	magic = info.magicattack;
	extend.attack_range = info.range;
	extend.max_mp = info.mp;
	extend.max_hp = info.hp;
}

void gactive_imp::HandleMessage(attack_msg& msg)
{
	XID src(1,1);
	A3DVECTOR pos(0,0,0);
    object_interface victim;
	gactive_imp imp(gvictim, false);
	ginfo.scope = 1;
	victim.Attach(&imp);
	msg.ainfo.level = gvictim.level;
	gwrapper.Attack(victim, src , pos, msg, 0 , 1);
	ginfo.scope = 0;
}

void gactive_imp::HandleMessage(enchant_msg& msg)
{
	XID src(1,1);
	A3DVECTOR pos(0,0,0);
    object_interface victim;
	gactive_imp imp(gvictim, false);
	ginfo.scope = 1;
	victim.Attach(&imp);
	msg.ainfo.level = gvictim.level;
	gwrapper.Attack(victim, src , pos, msg, 0 , 1);
	ginfo.scope = 0;
}



