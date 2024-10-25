#ifndef __SKILL_BASICINFO
#define __SKILL_BASICINFO

struct BasicInfo
{
	int cls;
	int level;
	int hp;
	int mp;
	int attack;
	int magicattack;
	float range;
	int defence;
	struct{
		int gold;
		int wood;
		int water;
		int fire;
		int earth;
	}resist;
};

extern BasicInfo fighter10;
extern BasicInfo fighter60;

extern BasicInfo wizard10;
extern BasicInfo wizard60;

extern BasicInfo beast10;
extern BasicInfo beast60;

extern BasicInfo fairy10;
extern BasicInfo fairy60;

struct SkillInfo
{
	int id;
	int damage;
	int magicdamage;
	float castrange;
	float range;
	float radius;
	int skill;
	int time;
	int mpcost;
	bool cast;
	int perform;
	float mptotal;
	int scope;  // 0: perfomer, 1: target
};

#endif

