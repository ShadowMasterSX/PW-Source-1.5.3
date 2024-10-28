#include <stdlib.h>
#include "string.h"
#include "world.h"
#include "moving_action.h"
#include "moving_action_env.h"
#include "actobject.h"
#include "worldmanager.h"

bool moving_action_env::StartAction(moving_action * pAction)
{
	if(_cur_action)
	{
		delete pAction;
		return false;
	}

	ASSERT(!_action_process);
	_cur_action = pAction;
	_cur_action->SetID(NextActionID());

	_action_process = true;
	if(!_cur_action->StartAction())
	{
		//一个瞬时的action 或者 启动失败, 不需要end action
		_action_process = false;
		delete _cur_action;
		_cur_action = NULL;
		return false;
	}
	_action_process = false;
	return true;
}
bool moving_action_env::ActionValid(int action_id)
{
	return _cur_action && _cur_action->GetID() == action_id;
}
void moving_action_env::EndAction()
{
	if(!_cur_action)
	{
		ASSERT(false);
		return;
	}

	_cur_action->EndAction();
	SafeDeleteCurAction();
	return;
}
void moving_action_env::RepeatAction()
{
	if(!_cur_action)
	{
		ASSERT(false);
		return;
	}

	_action_process = true;
	if(_imp->InNonMoveSession() || !_cur_action->RepeatAction())
	{
		_action_process = false;
		EndAction();
	}
	_action_process = false;
}
bool moving_action_env::ActionOnAttacked()
{
	if(!_cur_action)
	{
		ASSERT(false);
		return true;
	}
	return _cur_action->OnAttacked();
}
void moving_action_env::TryBreakAction()
{
	if(_cur_action && _cur_action->TerminateAction(false))
	{
		SafeDeleteCurAction();
	}
}
void moving_action_env::RestartAction()
{
	if(_cur_action)
	{
		if(!_cur_action->RestartAction())
		{
			EndAction();
		}
	}
}

bool moving_action_env::SafeDeleteCurAction()
{
	if(_action_process)
	{
		GLog::log(GLOG_ERR,"world[%d] action 存在嵌套释放",world_manager::GetWorldTag());
		return false;
	}
	else
	{
		delete _cur_action;
		_cur_action = NULL;
		return true;
	}
}

void moving_action_env::ClearAction()
{
	if(_cur_action)
	{
		_cur_action->TerminateAction();
		SafeDeleteCurAction();
	}
}
void moving_action_env::ReleaseAction()
{
	if(_cur_action)
	{
		SafeDeleteCurAction(); 
	}
}
