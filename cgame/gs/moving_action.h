#ifndef __ONLINEGAME_GS_MOVING_ACTTION_H__
#define __ONLINEGAME_GS_MOVING_ACTTION_H__

#include <amemobj.h>
#include <timer.h>
#include <amemory.h>
#include <vector.h>
#include <ASSERT.h>
#include <common/types.h>

#include "skillwrapper.h"

/*
	�ƶ��п�ִ�еĶ���������ʵ���ƶ�����
	����
	1 ͬʱֻ��ִ��һ��action,����action����
	2 ��ǰ���ڽ��з��ƶ�sessionʱ,�޷�����һ���µ�action
	3 action repeatʱ��������ڽ��з��ƶ�session,��action��ֹ
	4 actionִ�й����У��޷������չ�����session
	������
	Ŀǰֻ��player����ʹ��action
	skill action��������Ӧ��sessionʵ��һ��
	action�л�����ʱ������
 */

class gactive_imp;
class world;
class moving_action : public abase::ASmallObject
{
protected:
	gactive_imp * _imp;
	int _action_id;
	
	moving_action() : _imp(NULL), _action_id(-1){}
public:
	explicit moving_action(gactive_imp * imp) : _imp(imp), _action_id(-1){}
	virtual ~moving_action(){}

	inline void SetID(int action_id){ _action_id = action_id; }
	inline int GetID(){ return _action_id; }

	virtual bool StartAction() = 0;
	virtual bool EndAction() = 0;
	virtual bool RepeatAction() = 0;
	virtual bool TerminateAction(bool force = true) = 0;
	virtual bool RestartAction()  { return true;}
	virtual bool OnAttacked() { return false;}
};

class moving_timer_action : public moving_action, public abase::timer_task
{
protected:
	world * _plane;
	XID _self_id;

	moving_timer_action() : _plane(NULL){}
public:
	explicit moving_timer_action(gactive_imp * imp);
	virtual ~moving_timer_action()
	{
		if(_timer_index >= 0) RemoveTimer();
	}
	
	virtual bool StartAction() = 0;
	virtual bool EndAction()
	{
		RemoveTimer();
		return true;
	}
	virtual bool RepeatAction() = 0;
	virtual bool TerminateAction(bool force)
	{
		//Ĭ���ǲ��ܷ�ǿ���ж�
		if(!force) return false;
		EndAction();
		return true;
	}
protected:
	virtual void OnTimer(int index,int rtimes)
	{
		if(rtimes)
			SendRepeatMsg();
		else
			SendEndMsg();
	}
	void SendRepeatMsg();
	void SendEndMsg();
};

class moving_skill : public moving_timer_action
{
protected:
	SKILL::Data _data;
	int  _next_interval;
	bool _end_flag;
	char _force_attack;
	int  _skill_skip_time;
	abase::vector<XID, abase::fast_alloc<> > _target_list;

	moving_skill():_data(0){}
public:
	explicit moving_skill(gactive_imp * imp)
				:moving_timer_action(imp),_data(0),
				_next_interval(20),_end_flag(false),_force_attack(0), _skill_skip_time(0)
	{
	}
	void SetTarget(int skill_id, char force_attack,int target_num,int * targets);
	virtual bool StartAction();
	virtual bool RestartAction();
	virtual bool RepeatAction();
	virtual bool EndAction();
	virtual bool TerminateAction(bool force);

	virtual void OnTimer(int index,int rtimes);
	virtual bool OnAttacked();		// �������ʾҪ�ж�
};

class moving_instant_skill : public moving_action
{
protected:
	SKILL::Data _data;
	abase::vector<XID, abase::fast_alloc<> > _target_list;

	moving_instant_skill():_data(0){}
public:
	explicit moving_instant_skill(gactive_imp * imp)
				:moving_action(imp),_data(0)
	{}

	void SetTarget(int skill_id, char force_attack,int target_num,int * targets);
	virtual bool StartAction();
	virtual bool RepeatAction() { return false;}
	virtual bool EndAction() { return true;}
	virtual bool TerminateAction(bool force) { return true;}
};



#endif
