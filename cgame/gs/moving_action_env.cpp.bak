#include <stdlib.h>
#include "string.h"
#include "world.h"
#include "moving_action.h"
#include "moving_action_env.h"
#include "actobject.h"

bool moving_action_env::StartAction(moving_action * pAction)
{
	if(_cur_action)
	{
		delete pAction;
		return false;
	}

	_cur_action = pAction;
	_cur_action->SetID(NextActionID());

	if(!_cur_action->StartAction())
	{
		//一个瞬时的action 或者 启动失败, 不需要end action
		delete _cur_action;
		_cur_action = NULL;
		return false;
	}
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
	delete _cur_action;
	_cur_action = NULL;
	return;
}
void moving_action_env::RepeatAction()
{
	if(!_cur_action)
	{
		ASSERT(false);
		return;
	}

	if(_imp->InNonMoveSession() || !_cur_action->RepeatAction())
	{
		EndAction();
	}
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
		delete _cur_action;
		_cur_action = NULL;
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
void moving_action_env::ClearAction()
{
	if(_cur_action)
	{
		_cur_action->TerminateAction();
		delete _cur_action;
		_cur_action = NULL;
	}
}
void moving_action_env::ReleaseAction()
{
	if(_cur_action)
	{
		delete _cur_action;
		_cur_action = NULL;
	}
}
