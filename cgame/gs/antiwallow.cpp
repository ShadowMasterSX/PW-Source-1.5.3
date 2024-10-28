#include "antiwallow.h"

namespace anti_wallow
{



static punitive_param list[MAX_WALLOW_LEVEL] = {{0}};



const punitive_param & GetParam(size_t level)
{
	// Alien comentado abaixo para alterar exp,sp,etc
	/*if(level >= MAX_WALLOW_LEVEL)
	{
		level = MAX_WALLOW_LEVEL - 1;
	}*/
	return list[3];
}

void SetParam(size_t level, const punitive_param & param)
{
	if(level >= MAX_WALLOW_LEVEL)
	{
		level = MAX_WALLOW_LEVEL - 1;
	}
	list[level] = param;
	list[level].active = true;
}

void AdjustNormalExpSP(size_t level, int & exp, int & sp)
{
	const punitive_param & param = GetParam(level);
	exp = (int)(exp * param.exp + 0.5f);
	sp = (int)(sp * param.sp + 0.5f);
}

void AdjustTaskExpSP(size_t level, int & exp, int & sp)
{
	const punitive_param & param = GetParam(level);
	exp = (int)(exp * param.task_exp + 0.5f);
	sp = (int)(sp * param.task_sp + 0.5f);
}

void AdjustNormalMoneyItem(size_t level, float & money, float & item)
{
	const punitive_param & param = GetParam(level);
	money *= param.money;
	item *= param.item;
}

void AdjustTaskMoney(size_t level, int & money)
{
	const punitive_param & param = GetParam(level);
	money = (int)(money * param.task_money + 0.5f);
}
}

