#include "string.h"
#include "world.h"
#include "sitdown_filter.h"
#include "clstab.h"

DEFINE_SUBSTANCE(sit_down_filter,filter,CLS_FILTER_SITDOWN)

void 
sit_down_filter::OnRelease() 
{
	if(_timeout < 0)
	{
		_parent.ImpairScaleHPGen(STAYIN_BONUS);
		_parent.ImpairScaleMPGen(STAYIN_BONUS);
		_parent.UpdateHPMPGen();
	}
}

void 
sit_down_filter::Heartbeat(int tick)
{
	if(_timeout > 0)
	{
		--_timeout;
	}
	else if(!_timeout)
	{
		_timeout = -1;
		_parent.EnhanceScaleHPGen(STAYIN_BONUS);
		_parent.EnhanceScaleMPGen(STAYIN_BONUS);
		_parent.UpdateHPMPGen();
	}
}

