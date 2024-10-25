#include "templatedataman.h"


static int item_pickup[] = {3044,0, 11215,1, 3043,10, 27070,1, 27071,1}; 
static int skill_attack[] = {1374,132,18, 125,140,18, 299,69,17, 81,141,20, 1,65,0};

void TemplateDataMan::Init()
{
	for(int i=0; i<sizeof(item_pickup)/sizeof(int); i+=2)
	{
		int type = item_pickup[i];
		size_t pile_limit = item_pickup[i+1];
		pickup_items[type] = pile_limit;
	}
	//gather_mines.insert();
	//attack_npcs.insert();
	for(int i=0; i<sizeof(skill_attack)/sizeof(int); i+=3)
	{
		int id = skill_attack[i];
		skill_condition cond;
		cond.mp_cost = skill_attack[i+1];
		cond.range = skill_attack[i+2];
		attack_skills[id] = cond;
	}
}

bool TemplateDataMan::MatterCanPickup(int tid, size_t &pile_limit)
{ 
	std::map<int,size_t>::iterator it = pickup_items.find(tid);
	if(it == pickup_items.end()) return false;
	pile_limit = it->second;
	return true;
}

bool TemplateDataMan::IsAttackSkill(int id, int& mp_cost, float& range)
{
	std::map<int,skill_condition>::iterator it = attack_skills.find(id);
	if(it == attack_skills.end()) return false;
	skill_condition & cond = it->second;
	mp_cost = cond.mp_cost;
	range = cond.range;
	return true;
}
