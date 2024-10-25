#ifndef __VCLIENT_TASK_H__
#define __VCLIENT_TASK_H__

#include "ASSERT.h"
#include "types.h"
#include "config.h"

class PlayerImp;

enum TASK_TYPE
{
	TASK_TYPE_IDLE = 0,
	TASK_TYPE_MOVE,
	TASK_TYPE_NORMAL_ATTACK,
	TASK_TYPE_CAST_SKILL,
	TASK_TYPE_LOAD,
};

class task_basic
{
protected:
	PlayerImp * _imp;
	int _tick;

	task_basic():_imp(NULL),_tick(-1){}
public:
	task_basic(PlayerImp * imp, int tick=-1):_imp(imp),_tick(tick){}
	virtual bool StartTask() = 0;
	virtual void EndTask(){}
	virtual bool Tick()
	{
		if(_tick > 0)
		{
			--_tick;
			return _tick > 0;
		}
		return true;
	}
	virtual int GetTaskType() = 0;
};

class task_load : public task_basic
{
protected:
	task_load(){}
public:
	task_load(PlayerImp* imp,int delay):task_basic(imp,delay){}
	virtual bool StartTask(){ return true; };
	virtual bool Tick();
	virtual int GetTaskType(){ return TASK_TYPE_LOAD; }
};

class task_idle : public task_basic
{
protected:
	task_idle(){}
public:
	task_idle(PlayerImp * imp, int time):task_basic(imp)
	{
		if(time > 0) _tick = time/MILLISECOND_PER_TICK;
		else _tick = -1;
	}
	virtual bool StartTask(){ return true; };
	virtual bool Tick();
	virtual int GetTaskType(){ return TASK_TYPE_IDLE; }
};

class task_move : public task_basic
{
protected:
	A3DVECTOR _dest;
	int	_time;
	float _speed;
	char _mode;
	
	task_move(){}
public:
	task_move(PlayerImp * imp, A3DVECTOR& dest, int time, float speed, char mode)
		:task_basic(imp,time/MILLISECOND_PER_TICK),_dest(dest),_time(time),_speed(speed),_mode(mode)
		{ ASSERT(time >= 100); }
	virtual bool StartTask();
	virtual void EndTask(){}
	virtual int GetTaskType(){ return TASK_TYPE_MOVE; }
};

class task_stop_move : public task_move
{
protected:
	task_stop_move(){}
public:
	task_stop_move(PlayerImp * imp, A3DVECTOR& dest, int time, float speed, char mode)
		:task_move(imp, dest, time, speed, mode)
		{ ASSERT(time >= 100); }
	virtual bool StartTask();
	virtual void EndTask(){}
	virtual int GetTaskType(){ return TASK_TYPE_MOVE; }
};

class task_normal_attack : public task_basic
{
protected:
	task_normal_attack(){}
public:
	task_normal_attack(PlayerImp * imp)
		:task_basic(imp,-1){}
	virtual bool StartTask(){ return true; }
	virtual void EndTask(){}
	virtual int GetTaskType(){ return TASK_TYPE_NORMAL_ATTACK; }
};

class task_cast_skill : public task_basic
{
protected:
	task_cast_skill(){}
public:
	task_cast_skill(PlayerImp * imp)
		:task_basic(imp,-1){}
	virtual bool StartTask(){ return true; }
	virtual void EndTask(){}
	virtual int GetTaskType(){ return TASK_TYPE_CAST_SKILL; }
};

#endif
