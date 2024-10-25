#include "string.h"
#include "world.h"
#include "gt_award_filter.h"
#include "clstab.h"

DEFINE_SUBSTANCE(gt_award_filter,filter,CLS_FILTER_GTAWARD);


void gt_award_filter::OnAttach()
{
	_parent.InsertTeamVisibleState(243);
	_parent.EnhanceAttackDegree(_atk);
	_parent.EnhanceDefendDegree(_def);
}

void gt_award_filter::OnRelease()
{
	_parent.ImpairAttackDegree(_atk);
	_parent.ImpairDefendDegree(_def);
	_parent.RemoveTeamVisibleState(243);
}
