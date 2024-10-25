#ifndef __SKILL_MYWRAPPER
#define __SKILL_MYWRAPPER

#include "skill.h"
#include "skillwrapper.h"


using namespace GNET;
namespace GNET{

class MyWrapper : public SkillWrapper
{
public:
	void AddSkill(ID id, int level);
	void Clear();

	int GetLevel(ID id);
	int StartSkill( Data & , object_interface , const XID * , int , int & );
	bool Attack(object_interface target, const XID&, const A3DVECTOR&,const attack_msg& msg, int lid, bool invader);
	bool Attack(object_interface target, const XID&, const A3DVECTOR&,const enchant_msg& msg, int lid, bool invader );
};
}


#endif

