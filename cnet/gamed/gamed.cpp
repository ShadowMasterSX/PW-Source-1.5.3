#include "gproviderclient.hpp"
#include "conf.h"
#include "log.h"
#include "thread.h"
#include <iostream>
#include <unistd.h>

#include "types.h"
#include "obj_interface.h"
#include "factionlib.h"
#include "../gdbclient/db_if.h"
using namespace GNET;
void handle_user_cmd(int cs_index, int sid,int uid, const void * buf, size_t size) 
{
}
void user_enter_world(int cs_index,int sid,int uid) {}
void handle_user_msg(int cs_index,int sid, int uid, const void * msg, size_t size, char channel) {}
void user_kickout(int cs_index,int sid,int uid) {}
void user_lost_connection(int cs_index,int sid,int uid) {}
void user_login(int cs_index,int sid,int uid,const void * auth_buf, size_t size) {}
void trade_start(int trade_id,int role1,int role2,int localid1,int localid2) {}
void trade_end(int trade_id,int role1,int role2,bool need_read1,bool need_read2) {}
void switch_server_request(int link_id,int user_id, int localsid, int source,const void * , size_t ){}
void switch_server_cancel(int link_id,int user_id, int localsid, int source){}
void switch_server_timeout(int linkd, int userid, int local_sid) {}
void player_not_online(int userid, int link_id, int sid) {}
void get_task_data_reply(int taskid, int uid, const void * env_data, size_t env_size, const void * task_data, size_t task_size) {}
void get_task_data_timeout(int taskid, int uid, const void * env_data, size_t env_size) {};
void psvr_ongame_notify(int * start , size_t size,size_t step) {}
void psvr_offline_notify(int * start , size_t size,size_t step) {}
void gm_shutdown_server() {}
void player_cosmetic_result(int user_id, int ticket_index, int ticket_id, int result, unsigned int crc){}
void OnDeliveryConnected() { }
void notify_player_reward(int roleid, int reward, int param) {}
void player_end_sync(int role_id, unsigned int money, const GDB::itemlist & item_change_list, bool storesaved,bool cashsaved) {}
void battleground_start(int battle_id, int attacker, int defender,int end_time, int type, int map_type) { }
void player_enter_battleground(int role_id, int server_id,int world_tag, int battle_id) { }
namespace GNET
{
	void OnReportAttr( unsigned char attribute,const Octets& value ) { }
	void FactionLockPlayer(unsigned int tid,int roleid) { }
	void FactionUnLockPlayer(unsigned int tid,int roleid,const syncdata_t& syncdata) { }
	void ReceivePlayerFactionInfo(int roleid,unsigned int faction_id,char faction_role) { }
}
namespace GDB
{
	size_t convert_item(const GNET::RpcDataVector<GNET::GRoleInventory> & ivec,  itemdata * list , size_t size) { return 0;}
}
int main(int argc, char *argv[])
{
	if (argc != 3 || access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile " << "gameserver_id"<< std::endl;
		exit(-1);
	}

	Conf::GetInstance(argv[1]);
	Log::setprogname("gamed");
	/*
	{
		GProviderClient *manager = GProviderClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
	}
	*/
	{
		if ( !GProviderClient::Connect(atoi(argv[2]),1,-10000.0,10000.0,10000.0,-10000.0) )
			exit(0);
	}
	Thread::Pool::AddTask(PollIO::Task::GetInstance());
	Thread::Pool::Run();
	return 0;
}

