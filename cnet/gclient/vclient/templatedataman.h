#ifndef __VCLIENT_TEMPLATEDATAMAN_H__
#define __VCLIENT_TEMPLATEDATAMAN_H__

#include <set>
#include <map>
#include <stddef.h>

class TemplateDataMan
{
	std::map<int/*type*/,size_t/*pile_limit*/> pickup_items;	
	std::set<int> gather_mines;	
	std::set<int> attack_npcs;	
	struct skill_condition
	{
		int mp_cost;
		float range;
	};
	std::map<int, skill_condition> attack_skills;

public:
	void Init();
	bool MatterCanPickup(int tid, size_t &pile_limit);
	bool MatterCanGather(int tid){ return gather_mines.find(tid) != gather_mines.end();}
	bool NpcCanAttack(int tid){ return true; }
	bool IsAttackSkill(int id, int& mp_cost, float& range); 
};

#endif
