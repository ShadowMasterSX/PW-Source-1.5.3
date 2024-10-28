#ifndef __ONLINEGAME_GS_MOVINGACTTION_H__
#define __ONLINEGAME_GS_MOVINGACTTION_H__

class gactive_imp;
class moving_action;

struct moving_action_env
{
	gactive_imp * _imp;
	moving_action * _cur_action;
	int _next_action_id;	//action id 从0开始
	bool _action_process;

	moving_action_env(gactive_imp * imp) : _imp(imp), _cur_action(NULL), _next_action_id(0),_action_process(false) {}
	
	inline int NextActionID(){ return _next_action_id++; }
	moving_action * GetAction(){ return _cur_action; }
	bool StartAction(moving_action * pAction);
	bool ActionValid(int action_id);
	void EndAction();		//当前必须存在action!!!
	void RepeatAction();	//当前必须存在action!!!
	bool ActionOnAttacked();//当前必须存在action!!!
	void TryBreakAction();	//试图终止正在执行的action,可能失败
	void RestartAction();
	void ClearAction();		//强行终止正在执行的action
	void ReleaseAction();	//仅释放对象,不执行action逻辑 
private:	
	bool SafeDeleteCurAction();
};

#endif
