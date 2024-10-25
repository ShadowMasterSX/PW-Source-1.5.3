#ifndef __VCLIENT_VIRTUALCLIENT_H__
#define __VCLIENT_VIRTUALCLIENT_H__

#include "itimer.h"
#include "types.h"

class PlayerManager;
class MsgManager;
class MoveAgent;
class TemplateDataMan;
class VirtualClient : public GNET::IntervalTimer::Observer
{
	PlayerManager * player_manager;
	MsgManager * msg_manager;
	MoveAgent * move_agent;
	TemplateDataMan * template_data_man;
	int logic_tick_time;
	int logic_tick_range;
	int logic_tick_priority;
public:
	VirtualClient():player_manager(NULL),msg_manager(NULL),move_agent(NULL),template_data_man(NULL),logic_tick_time(1),logic_tick_range(1),logic_tick_priority(2){}
	~VirtualClient();
	void Init();
	void Quit();
	virtual bool Update();
	void Tick();
	
	PlayerManager * GetPlayerManager(){ return player_manager; }
	MsgManager * GetMsgManager(){ return msg_manager; }
	MoveAgent * GetMoveAgent(){ return move_agent; }
	TemplateDataMan * GetTemplateDataMan(){ return template_data_man; }
};

extern VirtualClient	g_virtualclient;
extern bool __PRINT_FLAG;
extern bool __PRINT_INFO_FLAG;
extern rect g_init_region;

#endif
