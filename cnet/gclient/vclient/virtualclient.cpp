#include "msg.h"
#include "msgmanager.h"
#include "playermanager.h"
#include "moveagent.h"
#include "templatedataman.h"
#include "virtualclient.h"
#include "config.h"
#include "../vclient_if.h"
#include "conf.h"
#include "dbgprt.h"

VirtualClient	g_virtualclient;
bool __PRINT_FLAG = false;
bool __PRINT_INFO_FLAG = false;
rect g_init_region;

VirtualClient::~VirtualClient()
{
	if(player_manager) delete player_manager;
	if(msg_manager) delete msg_manager;
	if(move_agent) delete move_agent;
	if(template_data_man) delete template_data_man;
}

void VirtualClient::Init()
{
	GNET::Conf *conf = GNET::Conf::GetInstance();
	__PRINT_FLAG = strcmp(conf->find("VirtualClient","output_debug_info").c_str(),"active") == 0;
	__PRINT_INFO_FLAG = strcmp(conf->find("VirtualClient","output_normal_info").c_str(),"active") == 0;
	std::string str = conf->find("VirtualClient","init_region");
	logic_tick_time = atoi(conf->find("VirtualClient","logic_tick_time").c_str());
	logic_tick_range = atoi(conf->find("VirtualClient","logic_tick_range").c_str());
	logic_tick_priority = atoi(conf->find("VirtualClient","logic_tick_priority").c_str());
	int ret = sscanf(str.c_str(), "(%f,%f)(%f,%f)", &g_init_region.left,&g_init_region.top,&g_init_region.right,&g_init_region.bottom);
	if(ret != 4 || g_init_region.left > g_init_region.right || g_init_region.top > g_init_region.bottom)
	{
		g_init_region.left = g_init_region.right = g_init_region.top = g_init_region.bottom = 1000.f;
	}
	__PRINTINFO("init region (%f,%f),(%f,%f)\n",g_init_region.left,g_init_region.top,g_init_region.right,g_init_region.bottom);

	player_manager = new PlayerManager();
	msg_manager = new MsgManager();
	move_agent = new MoveAgent();
	move_agent->Init();
	template_data_man = new TemplateDataMan();
	template_data_man->Init();
	//100ms update Ò»´Î
	GNET::IntervalTimer::Attach( this,MICROSECOND_PER_TICK/GNET::IntervalTimer::Resolution() );	
	
}

void VirtualClient::Quit()
{
	move_agent->Quit();
}

bool VirtualClient::Update()
{
	class virtual_client_tick : public GNET::Thread::Runnable
	{
		VirtualClient * _vclient;
		public:
		virtual_client_tick(VirtualClient * vclient):_vclient(vclient){}
		virtual void Run()
		{
			_vclient->Tick();
			delete this;
		}
	};

	class msg_tick : public GNET::Thread::Runnable
	{
		MsgManager * _msg;
		public:
		msg_tick(MsgManager * msg):_msg(msg),GNET::Thread::Runnable(1){}
		virtual void Run()
	 	{
			_msg->Tick();
			delete this;
		}
	};
	
	class player_tick : public GNET::Thread::Runnable
	{
		PlayerManager * _player;
		unsigned int _beg;
		public:
		player_tick(PlayerManager * player,unsigned int b,int prior):_player(player),_beg(b),GNET::Thread::Runnable(prior){}
		virtual void Run()
		{
			_player->SpliceTick(_beg);
			delete this;
		}
	};
	
	GNET::Thread::Pool::AddTask(new msg_tick(msg_manager)); 
	
	static int s_index = 0;
	for ( int n =0; n < logic_tick_time; ++n)
	{
		GNET::Thread::Pool::AddTask(new player_tick(player_manager,s_index++%logic_tick_range
					,logic_tick_priority));
	}


	return true;
}

void VirtualClient::Tick()
{
	player_manager->Tick();
	msg_manager->Tick();
}

//
namespace VCLIENT
{
void Init()
{
	g_virtualclient.Init();
}
	
void Quit()
{
	g_virtualclient.Quit();
}

void AddPlayer(int roleid)
{
	g_virtualclient.GetPlayerManager()->AddPlayer(roleid);
}

void RemovePlayer(int roleid)
{
	g_virtualclient.GetPlayerManager()->RemovePlayer(roleid);
}

void RecvS2CGameData(int roleid, void * buf, size_t size)
{
	g_virtualclient.GetPlayerManager()->DispatchS2CCmd(roleid,buf,size);
}

enum
{
	CONSOLECMD_PARAM_MAX = 5,
};
struct ConsoleCmd
{
	std::string cmd;
	size_t param_num;
	int param[CONSOLECMD_PARAM_MAX];
	ConsoleCmd():param_num(0){}
};

bool ParseConsoleCmd(char * buf, size_t size, ConsoleCmd& cc)
{
	cc.param_num = 0;
	std::string str(buf,size);
	std::string::size_type start = str.find_first_not_of(" \t");
	if(start == std::string::npos) return false;
	std::string::size_type end = str.find_first_of(" \t",start);
	if(end == std::string::npos)
	{
		cc.cmd = std::string(str,start);
		return true;
	}
	else
		cc.cmd = std::string(str,start,end-start);
	for(int i=0; i<CONSOLECMD_PARAM_MAX; i++)
	{
		start = str.find_first_not_of(" \t",end);
		if(start == std::string::npos) return true;
		end = str.find_first_of(" \t",start);
		if(end == std::string::npos)
		{
			std::string param_str(str,start);
			cc.param[cc.param_num++] = atoi(param_str.c_str());
			return true;
		}
		else
		{
			std::string param_str(str,start,end-start);
			cc.param[cc.param_num++] = atoi(param_str.c_str());
		}
	}
	return true;
}

void RecvConsoleCmd(int target, int source, char * buf, size_t size)
{
	MSG * msg = NULL;
	ConsoleCmd cc;
	if(!ParseConsoleCmd(buf,size,cc)) return;
	__PRINTF("RecvConsoleCmd [%s][%d][%d][%d][%d]\n",cc.cmd.c_str(), cc.param_num, cc.param[0],cc.param[1],cc.param[2]);

	if(cc.cmd == "switchmode")
	{
		if(cc.param_num == 1)
			msg = BuildMessage(MSG_SWITCH_MODE,target,cc.param[0]);
	}
	else if(cc.cmd == "follow")
	{
		if(cc.param_num == 0)
			msg = BuildMessage(MSG_FOLLOW_TARGET,target,source);
		else if(cc.param_num == 1)
			msg = BuildMessage(MSG_FOLLOW_TARGET,target,cc.param[0]);
	}
	else if(cc.cmd == "d_c2scmd")
	{
		if(cc.param_num >= 1)
			msg = BuildMessage(MSG_DEBUG_C2SCMD,target,cc.param[0],(cc.param_num-1)*sizeof(int),&cc.param[1]);
	}
	else if(cc.cmd == "forceattack")
	{
		if(cc.param_num == 0)
			msg = BuildMessage(MSG_CHANGE_FORCE_ATTACK,target,source);
		else if(cc.param_num == 1)
			msg = BuildMessage(MSG_CHANGE_FORCE_ATTACK,target,cc.param[0]);
	}
	else
		return;

	if(msg) g_virtualclient.GetMsgManager()->PostMessage(msg);
}
}
