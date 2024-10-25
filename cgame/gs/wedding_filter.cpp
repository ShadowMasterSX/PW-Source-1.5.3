#include "clstab.h"
#include "world.h"
#include "worldmanager.h"
#include "player_imp.h"
#include "wedding_filter.h"

DEFINE_SUBSTANCE(wedding_filter,filter,CLS_FILTER_WEDDING);

void wedding_filter::OnAttach()
{
	if(_parent.GetSelfID().id == _groom || _parent.GetSelfID().id == _bride)
		_parent.IncVisibleState(22);
	_parent.GetImpl()->_runner->enter_wedding_scene(_groom,_bride);
}

void wedding_filter::OnRelease() 
{
	if(_parent.GetSelfID().id == _groom || _parent.GetSelfID().id == _bride)
		_parent.DecVisibleState(22);
	_parent.GetImpl()->_runner->enter_wedding_scene(0,0);
}

void wedding_filter::Heartbeat(int tick)	
{
	if(_state == NORMAL)
	{
		if(_timeout > 0) _timeout --;
		if(_timeout <= 0)
		{
			if(!world_manager::GetInstance()->WeddingCheckOngoing(_groom,_bride,_scene))
			{
				_state = WAIT_ESCAPE;
				_timeout = 1;
			}
			else
				_timeout = 10;
		}
	}
	else if(_state == WAIT_ESCAPE)
	{
		if(_timeout > 0) _timeout --;
		if(_timeout <= 0)
		{
			gplayer_imp * pImp = (gplayer_imp*)_parent.GetImpl();
			pImp->LeaveAbnormalState();
			_timeout = 3;
			if(world_manager::GetSavePoint().tag > 0 
					&& world_manager::GetSavePoint().tag != world_manager::GetWorldTag())
			{
				pImp->LongJump(world_manager::GetSavePoint().pos,world_manager::GetSavePoint().tag);
			}
			else
			{
				pImp->LongJump(A3DVECTOR(0.f,300.f,0.f),1);
			}
		}
	}
}
