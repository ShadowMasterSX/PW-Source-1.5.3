#include "clstab.h"
#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "online_award_filter.h"

DEFINE_SUBSTANCE(online_award_exp_filter,filter,CLS_FILTER_ONLINE_AWARD_EXP);

void online_award_exp_filter::OnAttach()
{
	_parent.GetImpl()->_runner->toggle_online_award(_type, 1);
}

void online_award_exp_filter::OnRelease()
{
	_parent.GetImpl()->_runner->toggle_online_award(_type, 0);
}

void online_award_exp_filter::Heartbeat(int tick)
{
	if(++_counter >= _interval)
	{
		_counter  = 0;
		gplayer_imp * pImp = (gplayer_imp *)_parent.GetImpl();
		pImp->ReceiveCommonExp(_exp, int(_exp*0.226f));
		int rst = pImp->_online_award.SpendAwardTime(pImp, _type, _interval);
		if(rst <= 0)
		{
			_is_deleted = true;
		}
	}
}
