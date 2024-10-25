#include "breath_ctrl.h"
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "clstab.h"

void 
breath_ctrl::ChangeState(gplayer_imp * pImp, bool in_water)
{
	if(_breath_state == BREATH_NORMAL)
	{
		if(in_water)
		{
			_breath_state = BREATH_UNDERWATER;
			_breath = _breath_capacity;
			pImp->_runner->breath_data(_breath,_breath_capacity);
		}
	}
	else
	{
		if(!in_water)
		{
			_breath_state = BREATH_NORMAL;
			pImp->_runner->stop_dive();
		}
	}
}

void 
breath_ctrl::OnHeartbeat(gplayer_imp * pImp)
{
	if(_breath_state == BREATH_UNDERWATER && !_endless_breath)
	{
		//水下且没有无尽呼吸的时候才处理气条
		if(_breath <= 0)
		{
			attacker_info_t info ={XID(-1,-1),0,0,0,0,0};
			pImp->BeHurt(XID(-1,-1),info,DAMAGE_OUT_OF_BREATH,false,0);
		}
		else
		{
			_breath -= 2 + _point_adjust;
			if(_breath > _breath_capacity) _breath = _breath_capacity;
			pImp->_runner->breath_data(_breath,_breath_capacity);
		}
	}
}

void 
breath_ctrl::Save(archive & ar)
{
	ar << _breath << _breath_capacity << _breath_state << _endless_breath << _under_water << _point_adjust << _water_offset;
}

void 
breath_ctrl::Load(archive & ar)
{
	ar >> _breath >> _breath_capacity >> _breath_state >> _endless_breath >> _under_water >>_point_adjust >> _water_offset;
}


