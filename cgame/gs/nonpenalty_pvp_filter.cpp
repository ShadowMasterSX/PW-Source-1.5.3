#include "string.h"
#include "world.h"
#include "clstab.h"
#include "nonpenalty_pvp_filter.h"

DEFINE_SUBSTANCE(nonpenalty_pvp_filter,filter,CLS_FILTER_NONPENALTY_PVP);

void nonpenalty_pvp_filter::OnAttach()
{
	_parent.EnterNonpenaltyPVPState(true);
	_parent.InsertTeamVisibleState(234);
}

void nonpenalty_pvp_filter::OnRelease()
{
	_parent.EnterNonpenaltyPVPState(false);
	_parent.RemoveTeamVisibleState(234);
}
