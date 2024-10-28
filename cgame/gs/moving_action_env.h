#ifndef __ONLINEGAME_GS_MOVINGACTTION_H__
#define __ONLINEGAME_GS_MOVINGACTTION_H__

class gactive_imp;
class moving_action;

struct moving_action_env
{
	gactive_imp * _imp;
	moving_action * _cur_action;
	int _next_action_id;	//action id ��0��ʼ
	bool _action_process;

	moving_action_env(gactive_imp * imp) : _imp(imp), _cur_action(NULL), _next_action_id(0),_action_process(false) {}
	
	inline int NextActionID(){ return _next_action_id++; }
	moving_action * GetAction(){ return _cur_action; }
	bool StartAction(moving_action * pAction);
	bool ActionValid(int action_id);
	void EndAction();		//��ǰ�������action!!!
	void RepeatAction();	//��ǰ�������action!!!
	bool ActionOnAttacked();//��ǰ�������action!!!
	void TryBreakAction();	//��ͼ��ֹ����ִ�е�action,����ʧ��
	void RestartAction();
	void ClearAction();		//ǿ����ֹ����ִ�е�action
	void ReleaseAction();	//���ͷŶ���,��ִ��action�߼� 
private:	
	bool SafeDeleteCurAction();
};

#endif
