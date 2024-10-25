#include "task.h"
#include "player.h"

bool task_idle::Tick()
{
	if(_imp->HasNextTask()) return false;	
	return task_basic::Tick();
}

bool task_load::Tick()
{
	if(!task_basic::Tick())
	{
		_imp->get_all_data();
		return false;
	}
	return true;
}

bool task_move::StartTask()
{
	//直接就移动
	_imp->MoveTo(_dest);
	_imp->IncMoveSeq();
	_imp->player_move(_dest,_time,_speed,_mode);
 	return true;
}

bool task_stop_move::StartTask()
{
	//直接就移动
	_imp->MoveTo(_dest);
	_imp->IncMoveSeq();
	_imp->player_stop_move(_dest,_time,_speed,_mode);
 	return true;
}
