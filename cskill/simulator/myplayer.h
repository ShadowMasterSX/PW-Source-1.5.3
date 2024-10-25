#ifndef __SKILL_MYPLAYER
#define __SKILL_MYPLAYER

#include "skill.h"
#include "playerwrapper.h"

extern FILE* log_attack;

namespace GNET{

class MyPlayer : public PlayerWrapper
{
public:
	MyPlayer(object_interface o,TargetWrapper *t,SkillWrapper * w,Skill *s):PlayerWrapper(o,t,w,s) {}
	bool SetResurrect(bool)
	{
		fprintf(log_attack, "        复活    损失经验比例 %.2f\n", ratio);
		return true;
	}
	bool SetSpeedup(bool)
	{
		fprintf(log_attack, "        加速    比例%.2f  时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetIncdefence(bool)
	{
		fprintf(log_attack, "        增加物理防御    比例%.2f  时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetIncattack(bool)
	{
		fprintf(log_attack, "        增加物理攻击    比例%.2f  时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetIncmagic(bool)
	{
		fprintf(log_attack, "        增加法术攻击    比例%.2f  时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetAbsorbhp(bool)
	{
		fprintf(log_attack, "        吸取HP          比例%.2f  概率%.2f\n", ratio, probability);
		return true;
	}
	bool SetInchp(bool)
	{
		fprintf(log_attack, "        HP上限增加%.2f 时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetDechp(bool)
	{
		fprintf(log_attack, "        HP上限降低%.2f 时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetIncmp(bool)
	{
		fprintf(log_attack, "        MP上限增加%.2f 时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetIncdodge(bool)
	{
		fprintf(log_attack, "        增加躲避率 比例%.2f  时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetRepel(bool)
	{
		fprintf(log_attack, "        击退    距离%.2f  概率%.2f\n", value, probability);    
		return true;
	}
	bool SetBlind(bool)
	{
		fprintf(log_attack, "        目盲  时间%.2f  概率%.2f\n", time/1000, probability);    
		return true;
	}
	bool SetDecresist(bool)
	{
		return true;
	}
	bool SetIncresist(bool)
	{
		return true;
	}
	bool SetInchurt(bool)
	{
		fprintf(log_attack, "        伤害加倍 时间%.2f  比例%.2f\n", time/1000, ratio);    
		return true;
	}
	bool SetInaccurate(bool)
	{
		fprintf(log_attack, "        降低准确度 比例%.2f  时间%.2f\n", ratio, time/1000);
		return true;
	}
	bool SetMagicleak(bool)
	{
		fprintf(log_attack, "        魔力燃烧 时间%.2f  概率%.2f  总量%.2f\n", time/1000, probability, amount);    
		return true;
	}
	bool SetDecattack(bool) 
	{ 
		return true; 
	}
	bool SetDecdefence(bool)
	{
		fprintf(log_attack, "        降低防御力 时间%.2f  概率%.2f  比例%.2f\n", time/1000, probability, ratio);    
		return true;
	}
	bool SetFrighten(bool)
	{
		return true;
	}
	bool SetDizzy(bool)
	{
		fprintf(log_attack, "        昏迷 时间%.2f  概率%.2f\n", time/1000, probability);    
		return true;
	}
	bool SetSleep(bool)
	{
		fprintf(log_attack, "        睡眠 时间%.2f  概率%.2f\n", time/1000, probability);    
		return true;
	}
	bool SetFix(bool)
	{
		fprintf(log_attack, "        定身 时间%.2f  概率%.2f\n", time/1000, probability);    
		return true;
	}
	bool SetSealed(bool)
	{
		fprintf(log_attack, "        无法攻击 时间%.2f  概率%.2f\n", time/1000, probability);    
		return true;
	}
	bool SetSlow(bool)
	{
		fprintf(log_attack, "        降低移动速度 时间%.2f  概率%.2f  比例%.2f\n", time/1000, probability, ratio);    
		return true;
	}
	bool SetBleeding(bool)
	{
		fprintf(log_attack, "        放血 时间%.2f  概率%.2f  总量%d\n", time/1000, probability, (int)amount);    
		return true;
	}
	bool SetThunder(bool)
	{
		fprintf(log_attack, "        雷击 时间%.2f  概率%.2f  总量%d\n", time/1000, probability, (int)amount);    
		return true;
	}
	bool SetToxic(bool)
	{
		fprintf(log_attack, "        中毒 时间%.2f  概率%.2f  总量%d\n", time/1000, probability, (int)amount);    
		return true;
	}
	bool SetBurning(bool)
	{
		fprintf(log_attack, "        燃烧 时间%.2f  概率%.2f  总量%d\n", time/1000, probability, (int)amount);    
		return true;
	}
	bool SetFallen(bool)
	{
		fprintf(log_attack, "        落石 时间%.2f  概率%.2f  总量%d\n", time/1000, probability, (int)amount);    
		return true;
	}
	bool SetFeathershield(bool)
	{
		fprintf(log_attack, "        羽盾 时间%.2f  比例%.2f  数值%.2f\n", time/1000, ratio, value);    
		return true;
	}
	bool SetStoneshield(bool)
	{
		return true;
	}
	bool SetMagicshield(bool)
	{
		return true;
	}
	bool SetIceshield(bool)
	{
		return true;
	}
	bool SetRetort(bool)
	{
		fprintf(log_attack, "       反震  时间%.2f  比例%.2f\n", time/1000, ratio);    
		return true;
	}
	bool SetSlowpray(bool)
	{
		return true;
	}
	bool SetFastmpgen(bool)
	{
		return true;
	}
	bool SetFasthpgen(bool)
	{
		return true;
	}
	bool SetCrazy(bool)
	{
		return true;
	}
	bool SetTardy(bool)
	{
		return true;
	}
	bool SetIceblade(bool)
	{
		return true;
	}
};
}


#endif

