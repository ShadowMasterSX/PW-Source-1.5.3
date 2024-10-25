#include "string.h"
#include "world.h"
#include "npc_filter.h"
#include "clstab.h"
DEFINE_SUBSTANCE(npc_passive_filter,filter,CLS_FILTER_NPC_PASSIVE)

void npc_passive_filter::OnAttach()
{
	if((_is_aggressive = _parent.IsAggressive()))
	{
		_parent.SetAggressive(false);
	}
}
void npc_passive_filter::OnRelease()
{
	_parent.SetAggressive(_is_aggressive);
}

