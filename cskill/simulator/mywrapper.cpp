#include "mywrapper.h"
#include "myplayer.h"
#include "myobject.h"
#include "basicinfo.h"

extern SkillInfo ginfo;
extern std::vector<unsigned int> gdepends;
extern BasicInfo gvictim;
extern FILE* log_error;
extern std::map<int, std::string> gnames;

namespace GNET{

void MyWrapper::AddSkill(ID id, int level)
{
	PersistentData d(0,level);
	map[id] = d;
}

void MyWrapper::Clear()
{
	map.clear();
}
int MyWrapper::GetLevel(ID id)
{
	gdepends.push_back(id);
	if(ginfo.scope==1)
	{
		fprintf(log_error,"[%3d]!!_%-15s 击中效果计算与其他技能相关%s\n", id, gnames[ginfo.id].c_str(), 
				gnames[id].c_str()); 
	}
	return SkillWrapper::GetLevel(id);
}

int MyWrapper::StartSkill( Data & skilldata, object_interface player, const XID * target, int size, int & next_interval)
{   
	next_interval = -1;
	Skill * skill = Skill::Create(skilldata.id);
	if( NULL == skill )
		return -1; 

	TargetWrapper       w_target(target,size);
	MyPlayer w_player(player,&w_target,this,skill);
	skill->SetPlayer(&w_player);
	skill->SetTarget(&w_target);
	skill->SetLevel( GetLevel(skilldata.id) );
	skill->SetData(&skilldata);

	int ret = skill->FirstRun( next_interval, prayspeed );
	ginfo.castrange = skill->GetPraydistance();
	ginfo.mptotal = skill->GetStub()->GetMpcost(skill);
	if(ret>0)
	{
		player.SendClientMsgSkillCasting(size>0?*target:XID(0,0), skilldata.id, ret, skill->GetLevel());
	}

	skill->Destroy();
	return ret;
}

bool MyWrapper::Attack(object_interface victim, const XID& attacker, const A3DVECTOR& src,const attack_msg& msg, int lid, bool invader)
{
	int id = msg.attached_skill.skill;
	Skill * skill = Skill::Create(id);
	if( NULL == skill )
		return false;
	MyPlayer		w_victim(victim,NULL,this,skill);
	skill->SetVictim(&w_victim);

	skill->SetLevel( msg.attached_skill.level );
	skill->SetPerformerid( attacker );
	skill->SetMessage(&msg);
	skill->SetPerformerlid( lid );
	skill->SetPerformerpos( src );
	bool ret = skill->StateAttack();
	w_victim.SetEnmity( skill->GetEnmity() );
	if(skill->GetStub()->GetType()!=TYPE_ATTACK)
		victim.SendClientEnchantResult(attacker, msg.attached_skill.skill, msg.attached_skill.level, invader, msg.section);
	skill->Destroy();
	return ret;
}

bool MyWrapper::Attack(object_interface victim, const XID& attacker, const A3DVECTOR& src,const enchant_msg& msg, int lid, bool invader)
{
	Skill * skill = Skill::Create(msg.skill);
	if( NULL == skill )
		return false;

	MyPlayer		w_victim(victim,NULL,this,skill);
	skill->SetVictim(&w_victim);

	skill->SetLevel( msg.skill_level );
	skill->SetPerformerid( attacker );
	skill->SetMessage(&msg);
	skill->SetPerformerlid( lid );
	skill->SetPerformerpos( src );
	bool ret = skill->StateAttack();
	w_victim.SetEnmity( skill->GetEnmity() );
	if(skill->GetStub()->GetType()!=TYPE_ATTACK)
		victim.SendClientEnchantResult(attacker, msg.skill, msg.skill_level, invader, msg.section);
	skill->Destroy();
	return ret;
}
}
