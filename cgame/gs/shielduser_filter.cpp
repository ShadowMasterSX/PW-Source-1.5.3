#include "string.h"
#include "world.h"
#include "shielduser_filter.h"
#include "clstab.h"

DEFINE_SUBSTANCE(shielduser_filter,filter,CLS_FILTER_SHIELDUSER);

void shielduser_filter::OnAttach()
{
	//_parent.InsertTeamVisibleState(HSTATE_);
	//_parent.IncVisibleState(VSTATE_);
	_parent.EnhanceAttackDegree(1);
	_parent.EnhanceDefendDegree(1);
}

void shielduser_filter::OnRelease()
{
	//_parent.RemoveTeamVisibleState(HSTATE_);
	//_parent.DecVisibleState(VSTATE_);
	_parent.ImpairAttackDegree(1);
	_parent.ImpairDefendDegree(1);
}
