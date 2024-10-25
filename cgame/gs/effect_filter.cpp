#include "string.h"
#include "world.h"
#include "effect_filter.h"
#include "clstab.h"

DEFINE_SUBSTANCE(player_effect_filter,filter,CLS_FILTER_EFFECT)

void 
player_effect_filter::OnAttach()
{
	_parent.AddPlayerEffect(_effect);
}

void 
player_effect_filter::OnRelease()
{
	_parent.RemovePlayerEffect(_effect);
}

