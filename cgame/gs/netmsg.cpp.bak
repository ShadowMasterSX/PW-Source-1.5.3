#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <common/packetwrapper.h>
#include "world.h"
#include "config.h"
#include "netmsg.h"
#include "usermsg.h"
#include "userlogin.h"
#include "task/taskman.h"
#include "player_imp.h"
#include <common/protocol_imp.h>
#include <factionlib.h>
#include "instance/battleground_manager.h"
#include <strtok.h>
#include "instance/faction_world_ctrl.h"
#include "instance/countrybattle_manager.h"
#include "instance/trickbattle_manager.h"
#include "threadusage.h"
#include "gt_award_filter.h"
#include "global_controller.h"
#include "uniquedataclient.h"

inline static bool check_player(gplayer *pPlayer,int cs_index,int sid,int uid)
{
	return pPlayer->IsActived()     && pPlayer->cs_index == cs_index 
		&& pPlayer->cs_sid == sid 
		&& pPlayer->ID.id == uid;
}

inline static void single_trade_end(int trade_id, int role, bool need_read,int reason)
{	
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role || !pPlayer->IsActived() || !pPlayer->imp)  
	{
		return;
	}
	pPlayer->imp->TradeComplete(trade_id,reason,need_read);
}

void trade_end(int trade_id, int role1,int role2,bool need_read1,bool need_read2)
{
	ASSERT(role1 != role2 && role1 >0 && role2 > 0);
	if(role1 == role2 || role1 <= 0 ||  role2 <= 0) return;
	single_trade_end(trade_id,role1,need_read1,0);
	single_trade_end(trade_id,role2,need_read2,0);
}

void trade_start(int trade_id, int role1,int role2, int localid1,int localid2)
{
	ASSERT(role1 != role2 && role1 >0 && role2 > 0);
	try
	{
		if(role1 == role2 || role1 <= 0 ||  role2 <= 0) throw -1001;
		if(world_manager::GetWorldParam().forbid_trade) throw -1002;

		int index1,index2;
		gplayer *pPlayer1 = world_manager::GetInstance()->FindPlayer(role1,index1);
		gplayer *pPlayer2 = world_manager::GetInstance()->FindPlayer(role2,index2);
		if(!pPlayer1 || !pPlayer2 || index1 != index2)
		{
			//目标不存在或者不在同一个世界
			throw -1;
		}
		spin_doublelock keeper(pPlayer1->spinlock,pPlayer2->spinlock);

		if(pPlayer1->ID.id != role1 || pPlayer1->cs_sid != localid1) throw -3;
		if(pPlayer2->ID.id != role2 || pPlayer2->cs_sid != localid2) throw -4;
		if(!pPlayer1->IsActived() || pPlayer1->IsZombie() || pPlayer1->login_state != gplayer::LOGIN_OK) throw -5;
		if(!pPlayer2->IsActived() || pPlayer2->IsZombie() || pPlayer2->login_state != gplayer::LOGIN_OK) throw -6;

		if(pPlayer1->pos.squared_distance(pPlayer2->pos) > 10.f*10.f) throw -7;
		if(!pPlayer1->imp || !pPlayer2->imp) throw -10;

		if(!pPlayer1->imp->CanTrade(pPlayer2->ID)) throw -8;
		if(!pPlayer2->imp->CanTrade(pPlayer1->ID)) throw -9;
		pPlayer1->imp->StartTrade(trade_id,pPlayer2->ID);
		pPlayer2->imp->StartTrade(trade_id,pPlayer1->ID);
		GLog::log(GLOG_INFO,"玩家交易准备开始 用户%d <--> 用户%d, 交易id:%d",role1,role2,trade_id);

	}catch(int)
	{
		GMSV::ReplyTradeRequest(trade_id, role1,localid1,false);
	}
}

void player_not_online(int user_id, int link_id, int sid) 
{
	GLog::log(GLOG_INFO,"收到用户%d不在线的消息 %d %d",user_id, link_id, sid);
	//遇到这种情况的处理就是强行下线，而且不进行存盘操作
	int index1;
	gplayer * pPlayer = world_manager::GetInstance()->FindPlayer(user_id,index1);
	if(!pPlayer)
	{
		GLog::log(GLOG_INFO,"异常情况出现，Deliveryd速度好慢，去买彩票吧");
		//没有找到 合适的用户
		//正常情况下，这个用户应该存在的
		return;
	}
	spin_autolock keeper(pPlayer->spinlock);

	if(!pPlayer->IsActived() || pPlayer->ID.id != user_id  ||  
			pPlayer->cs_index != link_id || pPlayer->cs_sid != sid || !pPlayer->imp)
	{
		//这个用户可能正好已经消失了，所以直接返回，不进行处理
		return;
	}
	pPlayer->imp->PlayerForceOffline();
}

size_t handle_chatdata(int uid, const void * aux_data, size_t size, void * buffer, size_t len)
{
	//考虑根据具体数据来助理
	if(size < sizeof(short)) return 0;

	//简单的聊天处理策略
	int index;
	gplayer * pPlayer = world_manager::GetInstance()->FindPlayer(uid,index);
	if(!pPlayer) return 0;

	spin_autolock keeper(&pPlayer->spinlock);
	if(!pPlayer->IsActived() || pPlayer->ID.id != uid || pPlayer->b_disconnect || !pPlayer->imp)
	{
		//这种特殊情况直接忽略
		return 0;
	}
	gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);
	return pImp->TransformChatData(aux_data,size, buffer, len);
}

void handle_user_msg(int cs_index,int sid, int uid, const void * msg, size_t size, const void * aux_data, size_t size2, char channel)
{
	//简单的聊天处理策略
	int index;
	gplayer * pPlayer = world_manager::GetInstance()->FindPlayer(uid,index);
	if(!pPlayer) 
	{
		//要不要做特殊处理，因为它可能在另外一台服务器上
		GLog::log(GLOG_INFO,"user_msg::用户%d已经不在本游戏服务器中",uid);
		//现在先做下线处理
		GMSV::SendDisconnect(cs_index, uid, sid,0);
		return ;
	}

	spin_autolock keeper(&pPlayer->spinlock);
	if(!pPlayer->IsActived() || pPlayer->cs_index != cs_index 
				 || pPlayer->ID.id != uid 
				 || pPlayer->cs_sid != sid
				 || pPlayer->b_disconnect
				 || !pPlayer->imp)
	{
		//这种特殊情况直接忽略
		return ;
	}
	slice * pPiece = pPlayer->pPiece;
	if(pPiece == NULL)
	{
		//这种情况也直接忽略
		return ;
	}
	gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);

	char buffer[1024];
	int dsize = 0;
	if(size2) dsize = pImp->TransformChatData(aux_data,size2, buffer, sizeof(buffer));
	
	switch(channel)
	{
		case GMSV::CHAT_CHANNEL_LOCAL:
		case GMSV::CHAT_CHANNEL_TRADE:
		break;

		case GMSV::CHAT_CHANNEL_FARCRY:
		case GMSV::CHAT_CHANNEL_SUPERFARCRY:
			pImp->SendFarCryChat(channel,msg,size,buffer, dsize);
			return;

		case GMSV::CHAT_CHANNEL_WHISPER:
		case GMSV::CHAT_CHANNEL_FACTION: 
			return ;
		case GMSV::CHAT_CHANNEL_TEAM:
			pImp->SendTeamChat(channel,msg,size,buffer, dsize);
			return;

		case GMSV::CHAT_CHANNEL_BATTLE:
			pImp->SendBattleFactionChat(channel,msg,size,buffer, dsize);
			return;

		case GMSV::CHAT_CHANNEL_COUNTRY:
			pImp->SendCountryChat(channel,msg,size,buffer, dsize);
			return;
	}

	((gplayer_imp*)pPlayer->imp)->SendNormalChat(channel,msg,size,buffer, dsize);
	return ;
}

unsigned long long cmd_rdtsc_counter[1024];
unsigned long long cmd_number_counter[1024];

static __inline__ void RecordCmd(size_t cmd, unsigned long long l)
{
	if(cmd >= 1024) return;
	//考虑是否来个加锁，目前不需要
	cmd_number_counter[cmd] ++;
	cmd_rdtsc_counter[cmd] += l;
}

void handle_user_cmd(int cs_index,int sid,int uid,const void * buf, size_t size)
{
	if(size < sizeof(C2S::cmd_header))
	{
		//用户数据大小有错误的
		//GLog::log(GLOG_WARNING,"用户%d发来错误的数据",uid);
		GMSV::SendDisconnect(cs_index, uid, sid,0);
		GMSV::ReportCheater2Gacd(uid , 777, NULL,0);
		return ;
	}

	int index;
	gplayer * pPlayer = world_manager::GetInstance()->FindPlayer(uid,index);
	if(!pPlayer) 
	{
		//要不要做特殊处理，因为它可能在另外一台服务器上
		GLog::log(GLOG_INFO,"user_cmd::用户%d已经不在本游戏服务器中",uid);
		//现在先做下线处理 不处理了，以另外一种方式下线
		GMSV::SendDisconnect(cs_index, uid, sid,0);
		return ;
	}

	unsigned long long ll1,ll2;
	ll1 = rdtsc();
	int cmd = ((const C2S::cmd_header*)buf) -> cmd;

	pPlayer->Lock();
	if(!pPlayer->IsActived() || pPlayer->cs_index != cs_index 
				 || pPlayer->ID.id != uid 
				 || pPlayer->cs_sid != sid
				 || pPlayer->imp == NULL)
	{
		pPlayer->Unlock();
		return ;
	}
		
	if(pPlayer->login_state <= gplayer::WAITING_ENTER)
	{
		//验证一下是否可以取数据
		pPlayer->Unlock();
		GLog::log(GLOG_ERR,"user_cmd:在非符合的状态收到了客户端发来的数据 %d(外挂)",uid);
		return;
	}
	if(pPlayer->imp->DispatchCommand(cmd , buf, size) == 0)
	{
		ASSERT(pPlayer->spinlock && "锁怎么会被人乱解开呢？");
		if(!pPlayer->spinlock)
		{
			GLog::log(GLOG_ERR,"user_cmd:线程锁被意外解开了%d",uid);
		}
		//如果命令返回0才需要进行解锁
		pPlayer->Unlock();
	}
	else
	{
		//返回非0值就不需要解锁了,因为在DispatchCommand里面已经解开了锁
		ASSERT(pPlayer->spinlock == 0 && "出现的概率很低但是更可能是错误的情况");
	}

	ll2 = rdtsc();
	__PRINTF("player %6d command %4d use ---------------%8lld\n",uid, cmd, ll2 - ll1);
	RecordCmd(cmd, ll2 - ll1);
	
	//GLog::log(GLOG_INFO,"pcmd %2d [%d] start:%06ld",cmd,uid,tv.tv_usec);
	return;
}

void get_task_data_reply(int taskid, int uid, const void * env_data, size_t env_size, const void * task_data, size_t task_size)
{
	int world_index;
	gplayer * pPlayer = world_manager::GetInstance()->FindPlayer(uid,world_index);
	ASSERT(task_size == TASK_GLOBAL_DATA_SIZE);
	if(pPlayer)
	{
		spin_autolock keeper(pPlayer->spinlock);
		if(pPlayer->ID.id == uid && pPlayer->IsActived() && 
				pPlayer->login_state == gplayer::LOGIN_OK && pPlayer->imp)
		{
			//这是是否还要根据player_imp的其他状态作出一些判定...
			PlayerTaskInterface task_if(((gplayer_imp*)pPlayer->imp));
			OnTaskReceivedGlobalData(&task_if,taskid,(unsigned char*)task_data,env_data,env_size);
		}
	}
	//收到任务数据的回应
}

void get_task_data_timeout(int taskid, int uid, const void * env_data, size_t env_size)
{
	__PRINTF("收到全局任务数据的超时 \n");
}

void psvr_ongame_notify(int * start , size_t size,size_t step)
{
//	GLog::log(GLOG_INFO,"收到用户进入游戏的消息(%d):%d",*start,size);
	__PRINTF("收到用户进入游戏的消息(%d):%d\n",*start,size);
	world_manager::GetInstance()->BatchSetPlayerServer(start,size,step);
	
}

void psvr_offline_notify(int * start , size_t size,size_t step)
{
//	GLog::log(GLOG_INFO,"收到用户离开的消息(%d):%d",*start,size);
	__PRINTF("收到用户离开的消息(%d):%d\n",*start,size);
	ASSERT(size == 1);
	world_manager::GetInstance()->RemovePlayerServerIdx(*start);
	
}

template <int OFFLINE_CODE>
static void user_offline(int cs_index, int sid, int uid)
{
	GLog::log(GLOG_INFO,"用户断线了(%d):%d",OFFLINE_CODE,uid);
	int index;
	gplayer * pPlayer = world_manager::GetInstance()->FindPlayer(uid,index);
	if(pPlayer == NULL)
	{
		//如果已经不存在了，则同意断线要求
		GMSV::SendOfflineRe(cs_index,uid,sid,0);	// offline re
		return;
	}

	spin_autolock alock(pPlayer->spinlock);
	if(!check_player(pPlayer,cs_index,sid,uid))
	{
		GLog::log(GLOG_WARNING,"lost_connect(%d):用户%d已经不再服务器里了",OFFLINE_CODE,uid);
		GMSV::SendOfflineRe(cs_index,uid,sid,0);	// offline re
		return ;
	}
	if(pPlayer->login_state == gplayer::WAITING_LOGIN)
	{	
		//让player断线，不进行数据保存，直接释放对象
		ASSERT(pPlayer->pPiece == NULL);

		//这里不能够进行断线处理，因为数据库还没有处理完，一旦数据回来就可能会出错
		//所以只是设置一个断线标志
		pPlayer->b_disconnect = true;
		return ;
	}
	else if (pPlayer->login_state == gplayer::WAITING_ENTER)
	{
		//正在等待进入的阶段，直接退出
		__PRINTF("在enter_world阶段断线了%d\n",uid);
		ASSERT(pPlayer->pPiece == NULL);
		pPlayer->imp->_commander->Release();
		GMSV::SendOfflineRe(cs_index,uid,sid,0);	// offline re
		return ;
	}

	
	ASSERT(pPlayer->imp);
	//发来了断线消息，按照正常情况处理
	((gplayer_imp*)(pPlayer->imp))->LostConnection(OFFLINE_CODE);
}

void	user_lost_connection(int cs_index,int sid,int uid)
{
	user_offline<gplayer_imp::PLAYER_OFF_OFFLINE>(cs_index,sid,uid);
}

void	user_kickout(int cs_index,int sid,int uid)
{
	user_offline<gplayer_imp::PLAYER_OFF_KICKOUT>(cs_index,sid,uid);
}

void
switch_server_timeout(int link_id,int user_id,int localsid)
{
	GLog::log(GLOG_ERR,"用户%d转移服务器超时",user_id);
	//现在先不处理
	//ASSERT(false);
}

void switch_server_request(int link_id,int user_id, int localsid, int source,const void * key_buf, size_t key_size)
{
	GLog::log(GLOG_INFO,"用户%d(%d,%d)从%d请求转移至%d",user_id,link_id,localsid,source,world_manager::GetWorldTag());
	ASSERT(source != world_manager::GetWorldIndex());
	if(key_size != sizeof(instance_key))
	{
		GLog::log(GLOG_ERR,"用户%d转移请求KEY大小错误",user_id);
		return;
	}
	instance_key & nkey = *(instance_key*)key_buf;
	world_manager::GetInstance()->HandleSwitchRequest(link_id,user_id,localsid,source,nkey);
}

void switch_server_cancel(int link_id,int user_id, int localsid, int )
{
	world_manager::GetInstance()->SwitchServerCancel(link_id,user_id,localsid);
}


void faction_trade_lock(int trade_id, int roleid)
{
	try
	{
		int index;
		gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
		if(!pPlayer) 
		{
			//目标不存在
			throw -1;
		}
		spin_autolock keeper(pPlayer->spinlock);

		if(pPlayer->ID.id != roleid ) throw -2;
		if(!pPlayer->IsActived() || pPlayer->IsZombie() || pPlayer->login_state!=gplayer::LOGIN_OK) throw -3;
		if(!pPlayer->imp) throw -4;
		//FactionOPRequest不会修改玩家的数据库，putmask设置为ALL
		if(!pPlayer->imp->StartFactionTrade(trade_id,0,DBMASK_PUT_ALL)) throw -5;
	}catch(int)
	{
		GNET::syncdata_t data(0,0);
		GNET::SendFactionLockResponse(-1,trade_id,roleid,data);
	}
}

inline void faction_trade_end(int trade_id, int role, const GNET::syncdata_t & data)
{	
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	pPlayer->imp->FactionTradeComplete(trade_id,data);
}

void player_end_sync(int role, unsigned int money, GDB::itemlist const& item_change,bool is_write_trashbox, bool is_write_shoplog)
{
	__PRINTF("%d通知内部交易结束受到\n",role);
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	pPlayer->imp->SyncTradeComplete(0,money, item_change,is_write_trashbox, is_write_shoplog);
}


void player_cosmetic_result(int role, int ticket_inv_idx, int ticket_id, int result, unsigned int crc)
{
	__PRINTF("收到整容成功的消息\n");
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->CosmeticSuccess(ticket_inv_idx, ticket_id, result, crc);
}

void notify_player_reward(int role, int reward, int reward2, int param, int rewardmask)
{
	//donothing
	__PRINTF("recv player reward %d %d %d\n",role, reward, param);
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp
			|| (pPlayer->login_state!=gplayer::LOGIN_OK && pPlayer->login_state != gplayer::WAITING_ENTER))
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->SetSpecailTaskAward(reward,reward2,param,rewardmask);
	if(reward)
	{
		GLog::log(GLOG_INFO,"用户%d存在任务奖%d %d",role,reward,param);
	}
}

namespace GNET
{
bool FactionRenameVerify(int roleid ,int fid,const void* name,size_t len)
{
	__PRINTF("player rename faction\n");
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return false;
	spin_autolock keeper(pPlayer->spinlock);

	gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;
	if(!pImp) return false;

	int ipos = -1, itype = 0;
	if((ipos = pImp->GetItemPos(FACTION_RENAME_ITEM_1)) != -1)
		itype = FACTION_RENAME_ITEM_1;	
	else if((ipos = pImp->GetItemPos(FACTION_RENAME_ITEM_2)) != -1)
		itype = FACTION_RENAME_ITEM_2;	
	else
	{
		GLog::log(GLOG_ERR,"用户%d非法改名帮派",roleid);
		return false;
	}

	object_interface obj_if(pImp);

	if(!FactionRenameRespond(roleid,fid,itype,1,ipos,name,len,obj_if))	
	{
		GLog::log(GLOG_ERR,"角色%d帮派改名同步数据失败",roleid); 
		return false;
	}

	GLog::log(GLOG_INFO,"用户%d改名帮派使用物品%d",roleid,itype);
	return true;
}

void FactionLockPlayer(unsigned int tid,int roleid)
{
	__PRINTF("faction lock player\n");
	faction_trade_lock(tid,roleid);
}

void FactionUnLockPlayer(unsigned int tid,int roleid,const syncdata_t& syncdata)
{
	__PRINTF("faction unlock player\n");
	faction_trade_end(tid, roleid,syncdata);
}
void ReceivePlayerFactionInfo(int roleid,unsigned int faction_id,char faction_role,unsigned char faction_pvp_mask)
{
	__PRINTF("recv player faction info\n");
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid || !pPlayer->IsActived() || !pPlayer->imp 
			|| (pPlayer->login_state!=gplayer::LOGIN_OK && pPlayer->login_state!=gplayer::WAITING_ENTER))
	{
		return;
	}
	if(pPlayer->id_mafia != (int)faction_id || pPlayer->rank_mafia != faction_role || pPlayer->mafia_pvp_mask != faction_pvp_mask)
	{
		//帮派信息有变
		pPlayer->imp->UpdateMafiaInfo(faction_id, faction_role, faction_pvp_mask);
	}
}
void ReceivePlayerFactionRelation(int roleid,unsigned int faction_id,int* alliance,size_t asize,int* hostile, size_t hsize)
{
	__PRINTF("recv player faction relation\n");
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid || !pPlayer->IsActived() || !pPlayer->imp || pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	if(pPlayer->id_mafia != (int)faction_id)
	{
		return;
	}
	pPlayer->imp->UpdateFactionRelation(alliance,asize,hostile,hsize,true);
}
};

void gm_shutdown_server()
{
	world_manager::GetInstance()->ShutDown();
}

void battleground_start(int battle_id, int attacker, int defender,int end_time, int type, int map_type)
{
	battle_ground_param param;
	memset(&param,0,sizeof(param));
	param.battle_id = battle_id;
	param.attacker = attacker;
	param.defender = defender;
	param.player_count = 10; 	//此值已作废，使用配置文件中的设定
	param.end_timestamp = end_time;
	if(world_manager::GetInstance()->CreateBattleGround(param))
	{
		GNET::ResponseBattleStart(battle_id,0);
	}
	else
	{
		GNET::ResponseBattleStart(battle_id,-1);
	}
}

void player_enter_battleground(int role, int server_id, int world_tag, int battle_id)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->EnterBattleground(world_tag, battle_id);
}

void OnDeliveryConnected()
{
	world_manager::GetInstance()->OnDeliveryConnected();
}

bool gm_control_command(int target_tag, const char * buf)
{
	if(target_tag != -1 && target_tag != world_manager::GetWorldTag()) return false;

	abase::strtok tok(buf, " \t");
	const char * cmd = tok.token();
	if(!cmd) return false;

	if(strcmp(cmd, "active_npc_generator") == 0)
	{
		const char * arg = tok.token();
		if(arg)
		{
			int value = atoi(arg);
			return world_manager::GetInstance()->TriggerSpawn(value);
		}
		return false;
	}
	else if(strcmp(cmd,"cancel_npc_generator") == 0)
	{
		const char * arg = tok.token();
		if(arg)
		{
			int value = atoi(arg);
			return world_manager::GetInstance()->ClearSpawn(value);
		}
		return false;
	}
	else
	{
		return false;
	}
}

void player_modify_cashused(int role_id, int cash_used)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role_id,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role_id || !pPlayer->IsActived() || !pPlayer->imp)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->DeliveryNotifyModifyCashUsed(cash_used);
}

void player_cash_notify(int role, int cash_plus_used)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->DeliveryNotifyCash(cash_plus_used);
}

void player_add_cash_notify(int role)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;

	object_interface obj_if(pImp);
	if(obj_if.TradeLockPlayer(0,DBMASK_PUT_ALL) != 0)
	{
		__PRINTF("收到元宝数已变的通知，但玩家(%d)状态不对, 无法更新元宝\n",role);
		return;
	}

	class GetCashTotalResult : public abase::ASmallObject, public GDB::Result
	{
		int _roleid;
	public:
		GetCashTotalResult(int roleid):_roleid(roleid){}
		virtual void OnTimeOut()
		{
			__PRINTF("玩家(%d)更新元宝超时了\n",_roleid);
			OnGetCashTotal(-1);
		}
		virtual void OnFailed()
		{
			__PRINTF("玩家(%d)更新元宝失败了\n",_roleid);
			OnGetCashTotal(-1);
		}
		virtual void OnGetCashTotal(int cash_total)
		{
			int index;
			gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(_roleid,index);
			if(!pPlayer) return ; 
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _roleid||!pPlayer->IsActived()||!pPlayer->imp)
			{
				return;
			}
			gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;

			if(cash_total >= 0)
			{
				int old_cash = pImp->GetMallCash();
				pImp->DeliveryNotifyCash(cash_total);	
				GLog::log(GLOG_INFO,"用户%d更新元宝完成，元宝改变为%d",_roleid, pImp->GetMallCash()-old_cash);
			}

			object_interface obj_if(pImp);
			obj_if.TradeUnLockPlayer();
			delete this;
		}
	};
	GDB::get_cash_total(pImp->GetDBUserId(),new GetCashTotalResult(role));
}

void player_dividend_notify(int role, int dividend)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->_dividend_mall_info.IncDividendAdd(dividend);
	pImp->_runner->player_dividend(pImp->_dividend_mall_info.GetDividend());
}

bool query_player_level(int role, int & level, int & reputation)
{
	level = 0;
	reputation = 0;
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return false;
	spin_autolock keeper(pPlayer->spinlock);
	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp)
	{
		return false;	
	} 
	level = pPlayer->base_info.level;	
	reputation = ((gplayer_imp*)pPlayer->imp)->GetReputation();
	return true;
}

void report_cheater(int role, int cheattype, const void *cheatinfo, size_t size)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp || pPlayer->login_state != gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->FindCheater(cheattype,true);
}

void wallow_control(int userid, int rate, std::map<int,int> & data, int msg)
{
	__PRINTF("recv player %d wallow control %d\n", userid, rate);
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(userid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != userid ||!pPlayer->IsActived()||!pPlayer->imp || pPlayer->login_state != gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->WallowControl(rate, data[1], data[2], data[3], msg);
}

void acquestion_ret(int userid, int ret) // ret: 0 正确, 1 错误, 2 超时
{
	if(ret != 0) return ;//正确才处理
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(userid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != userid ||!pPlayer->IsActived()||!pPlayer->imp || pPlayer->login_state != gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->QuestionBonus();

}

void player_rename_ret(int roleid, const void *new_name, size_t name_len, int ret)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);
	
	if(pPlayer->ID.id != roleid ||!pPlayer->IsActived()||!pPlayer->imp || pPlayer->login_state != gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->PlayerRenameRet(new_name, name_len, ret);
}

static void single_divorce(int role)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	if(pImp->IsMarried())
	{
		pImp->SetSpouse(0);
		if(pPlayer->login_state == gplayer::LOGIN_OK
				&& pImp->_plane) {
			pImp->_runner->player_change_spouse(0);
		}
	}
}

void player_on_divorce(int id1, int id2)
{
	single_divorce(id1);
	single_divorce(id2);
}

bool player_billing_approved(int userid, int itemid, int itemnum, int expire_time, int cost)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(userid,index);
	if(!pPlayer) return false;
	spin_autolock keeper(pPlayer->spinlock);
	if(pPlayer->ID.id != userid ||!pPlayer->IsActived()||!pPlayer->imp || pPlayer->login_state != gplayer::LOGIN_OK)
	{
		return false;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	return pImp->ForeignDoShoppingStep2(itemid, itemnum , expire_time, cost);
	
}

bool generate_item_for_delivery(int id, GDB::itemdata & data)
{
	element_data::item_tag_t tag = {element_data::IMT_CREATE,0}; 
	item_data * pData = world_manager::GetDataMan().generate_item_for_drop(id,&tag,sizeof(tag));
	if(!pData) return false;
	if(pData->pile_limit == 0) return false;
	data.id = pData->type;
	data.index = -1;
	data.count = 1;
	data.max_count = pData->pile_limit;
	data.guid1 = pData->guid.guid1;
	data.guid2 = pData->guid.guid2;
	data.mask = pData->equip_mask;
	data.proctype = pData->proc_type;
	data.expire_date = pData->expire_date;
	if(pData->content_length > 0)
	{
		data.data = new char[pData->content_length];
		if(data.data == NULL) return false;
		memcpy((void*)data.data, pData->item_content, pData->content_length);
		data.size = pData->content_length;
	}
	else
	{
		data.data = NULL;
		data.size = 0;
	}
	FreeItem(pData);
	world_manager::TestCashItemGenerated(id, 1);
	return true;
}

bool get_faction_fortress_create_cost(int* cost, size_t& size)
{
	DATA_TYPE dt;
	FACTION_FORTRESS_CONFIG * cfg = (FACTION_FORTRESS_CONFIG *)world_manager::GetDataMan().get_data_ptr(FACTION_FORTRESS_CONFIG_ID,ID_SPACE_CONFIG,dt);
	if(!cfg || dt != DT_FACTION_FORTRESS_CONFIG) return false;
	if(size < sizeof(cfg->require_item)/sizeof(cfg->require_item[0])*2) return false;
	size = 0;
	for(size_t i=0; i<sizeof(cfg->require_item)/sizeof(cfg->require_item[0]); i++)
	{
		if(cfg->require_item[i].id <= 0 || cfg->require_item[i].count <= 0) break;
		cost[size++] = cfg->require_item[i].id;
		cost[size++] = cfg->require_item[i].count;
	}
	return true;
}

bool get_faction_fortress_initial_value(int* technology, size_t& tsize, int* material, size_t& msize, int* building, size_t& bsize)
{
	DATA_TYPE dt;
	FACTION_FORTRESS_CONFIG * cfg = (FACTION_FORTRESS_CONFIG *)world_manager::GetDataMan().get_data_ptr(FACTION_FORTRESS_CONFIG_ID,ID_SPACE_CONFIG,dt);
	if(!cfg || dt != DT_FACTION_FORTRESS_CONFIG) return false;
	if(tsize < faction_world_ctrl::TECHNOLOGY_COUNT || msize < faction_world_ctrl::MATERIAL_COUNT || bsize < sizeof(cfg->init_building)/sizeof(int)*2)  return false;
	memset(technology,0,sizeof(int)*faction_world_ctrl::TECHNOLOGY_COUNT);
	tsize = faction_world_ctrl::TECHNOLOGY_COUNT;
	memset(material,0,sizeof(int)*faction_world_ctrl::MATERIAL_COUNT);
	msize = faction_world_ctrl::MATERIAL_COUNT;
	bsize = 0;
	for(size_t i=0; i<sizeof(cfg->init_building)/sizeof(int) && i<faction_world_ctrl::BUILDING_MAX; i++)
	{
		if(cfg->init_building[i] <= 0) break;		
		building[bsize++] = cfg->init_building[i];
		building[bsize++] = 0;
	}
	return true;
}

bool notify_faction_fortress_data(GNET::faction_fortress_data2 * data2)
{
	return world_manager::GetInstance()->NotifyFactionData(data2);
}

void player_enter_faction_fortress(int role, int dst_world_tag, int dst_factionid)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->EnterFactionFortress(dst_world_tag, dst_factionid);
}

void RecvFactionCongregateRequest(int factionid, int roleid, int sponsor, void * data, size_t size)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	if(pPlayer->id_mafia != factionid)
	{
		return;
	}

	struct _data
	{
		world_pos wpos;
		int level_req;
		int sec_level_req;
        int reincarnation_times_req;
	};
	if(size != sizeof(struct _data)) return;
	struct _data * pData = (struct _data *)data;
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->RecvCongregateRequest(gplayer_imp::CONGREGATE_TYPE_FACTION, sponsor, pData->wpos.tag, pData->wpos.pos, pData->level_req, pData->sec_level_req, pData->reincarnation_times_req);

}
void UpdateForceGlobalData(int force_id, int player_count, int development, int construction, int activity, int activity_level)
{
	world_manager::GetForceGlobalDataMan().SetData(force_id,player_count,development,construction,activity,activity_level);
}

void player_get_aumail_task(int roleid, int level,char ex_reward)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid ||!pPlayer->IsActived()||!pPlayer->imp || pPlayer->login_state != gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->GetAUMailTask(level,ex_reward);
}

bool player_join_country(int role, int country_id, int country_expiretime, int major_strength, int minor_strength, int world_tag, float posx, float posy, float posz)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return false;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return false;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	return pImp->CountryJoinStep1(country_id, country_expiretime, major_strength, minor_strength, world_tag, A3DVECTOR(posx,posy,posz));
}

void notify_country_battle_config(GMSV::CBConfig * config)
{
	world_manager::GetInstance()->NotifyCountryBattleConfig(config);
}

void country_battle_start(int battle_id, int attacker, int defender,int player_limit, int end_time, int attacker_total, int defender_total, int max_total)
{
	country_battle_param param;
	memset(&param,0,sizeof(param));
	param.battle_id = battle_id;
	param.attacker = attacker;
	param.defender = defender;
	param.player_count = player_limit;
	param.end_timestamp = end_time;
	param.attacker_total = attacker_total;
	param.defender_total = defender_total;
	param.max_total = max_total;
	if(world_manager::GetInstance()->CreateCountryBattle(param))
	{
		GMSV::ResponseCountryBattleStart(battle_id, world_manager::GetWorldTag(), 0, defender, attacker);
	}
	else
	{
		GMSV::ResponseCountryBattleStart(battle_id, world_manager::GetWorldTag(), -1, defender, attacker);
	}
}

void player_enter_country_battle(int role, int world_tag, int battle_id)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->EnterCountryBattle(world_tag, battle_id);
}

void player_country_territory_move(int role, int world_tag, float posx, float posy, float posz, bool capital)
{
	if(world_tag != world_manager::GetWorldTag()) return;
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->CountryTerritoryMove(A3DVECTOR(posx,posy,posz),capital);
}

void country_battle_destory_instance(int battleid, int world_tag)
{
	if(world_tag != world_manager::GetWorldTag()) return;
	world_manager::GetInstance()->DestroyCountryBattle(battleid);
}

void thread_usage_stat(const char * ident)
{
	ThreadUsage::StatSelf(ident);	
}

bool query_player_info(int role, char * name, size_t& name_len, int& level, int& sec_level, int& reputation, int& create_time, int& factionid, int itemid1, int& itemcount1, int itemid2, int& itemcount2, int itemid3, int& itemcount3, int& reincarn_time, int& realm_level )
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return false;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return false;
	}

	gplayer_imp * pImp = (gplayer_imp *)pPlayer->imp;
	size_t len = 0;
	const char * pName = (const char *)pImp->GetPlayerName(len);
	ASSERT(name_len >= len);
	name_len = len;
	memcpy(name, pName, len);
	level = pImp->_basic.level;
	sec_level = pImp->_basic.sec_level;
	reputation = pImp->GetReputation();
	create_time = pImp->GetCreateTime();
	factionid = pImp->OI_GetMafiaID();
	if(itemid1) itemcount1 = pImp->GetItemCount(itemid1);
	if(itemid2) itemcount2 = pImp->GetItemCount(itemid2);
	if(itemid3) itemcount3 = pImp->GetItemCount(itemid3);
	reincarn_time = pImp->GetReincarnationTimes();
	realm_level = pImp->GetRealmLevel();
	return true;
}

 void player_enter_leave_gt(int op,int roleid)
 {
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);
	if(!pPlayer) return;
	spin_autolock keeper(pPlayer->spinlock);
	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp * pImp = (gplayer_imp *)pPlayer->imp;
	if(op)
	{
		DATA_TYPE dt;
		GT_CONFIG * cfg = (GT_CONFIG *)world_manager::GetDataMan().get_data_ptr(GT_CONFIG_ID,ID_SPACE_CONFIG,dt);
		if(!cfg || dt != DT_GT_CONFIG) return;
		if(cfg->inc_attack_degree < 0 || cfg->inc_defend_degree < 0) return;
		pImp->_filters.AddFilter(new gt_award_filter(pImp,cfg->inc_attack_degree,cfg->inc_defend_degree));
	}
	else
	{
		pImp->_filters.RemoveFilter(FILTER_INDEX_GTAWARD);
	}
 }

void player_safe_lock_changed(int role, int locktime, int max_locktime)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_controller * pCtrl = (gplayer_controller *) pPlayer->imp->_commander;
	pCtrl->SetSafeLock(locktime, max_locktime);
}

void player_change_ds(int role, int flag)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp *) pPlayer->imp;
	pImp->PlayerChangeDSLogout(flag);
}
void notify_cash_money_exchange_rate(bool open, int rate)
{
	world_manager::GetGlobalController().SetCashMoneyExchangeRate(open, rate);
}
void king_notify(int role, int end_time)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp *) pPlayer->imp;
	if(!pPlayer->IsKing()) pImp->SetKing(true, end_time);
}
void OnTouchPointQuery(int roleid,int64_t income,int64_t remain)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnTouchPointQuery(income,remain);
}

void OnTouchPointCost(int roleid,int64_t orderid,unsigned int cost,int64_t income,int64_t remain,int retcode)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnTouchPointCost(orderid,cost,income,remain,retcode);

}

void OnAuAddupMoneyQuery(int roleid,int64_t addupmoney)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnAuAddupMoneyQuery(addupmoney);
}

void OnGiftCodeRedeem(int roleid,void* cn,size_t cnsz,int type,int parenttype,int retcode)
{
	if(cnsz != player_giftcard::GIFT_CARDNUMBER_LEN)
	{
		GLog::log(GLOG_ERR,"role%d giftcode length%d err",roleid,cnsz);
		return;
	}

	char cardnumber[player_giftcard::GIFT_CARDNUMBER_LEN];
	memcpy(cardnumber,cn,cnsz);
	
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);
	
	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnGiftCardRedeem(cardnumber,type,parenttype,retcode);
}

void OnUniqueDataLoad(int key,int type,int version, const void* p,size_t sz)
{
	world_manager::GetInstance()->GetUniqueDataMan().OnDataLoad(key,type,version,p,sz);
}

void OnUniqueDataLoadFinish()
{
	world_manager::GetInstance()->GetUniqueDataMan().OnLoadFinish();
}

void OnUniqueDataClose()
{
	world_manager::GetInstance()->GetUniqueDataMan().OnSystemClose();
}

void OnUniqueDataModify(int worldtag, int key, int type, const void* val, size_t sz, const void* oldval, size_t osz, int retcode, int version)
{
	world_manager::GetInstance()->GetUniqueDataMan().OnDataModify(worldtag,key,type,val,sz,oldval,osz,retcode,version);
}

void trick_battle_start(int battle_id, int player_limit, int end_time)
{
	trick_battle_param param;
	memset(&param,0,sizeof(param));
	param.battle_id = battle_id;
	param.player_count = player_limit;
	param.end_timestamp = end_time;
	if(world_manager::GetInstance()->CreateTrickBattle(param))
	{
		GMSV::ResponseTrickBattleStart(battle_id, world_manager::GetWorldTag(), 0);
	}
	else
	{
		GMSV::ResponseTrickBattleStart(battle_id, world_manager::GetWorldTag(), -1);
	}
}

void player_enter_trick_battle(int role, int world_tag, int battle_id, int chariot)
{
	int index;
	gplayer *pPlayer = world_manager::GetInstance()->FindPlayer(role,index);
	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != role||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}
	gplayer_imp * pImp = (gplayer_imp*)pPlayer->imp;
	pImp->EnterTrickBattleStep1(world_tag, battle_id, chariot);
}

void autoteam_player_ready(int roleid, int leader_id)
{
	int index;
	gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnAutoTeamPlayerReady(leader_id);
}

void autoteam_compose_failed(int roleid, int leader_id)
{
	int index;
	gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnAutoTeamComposeFailed(leader_id);
}

void autoteam_compose_start(int goal_id, int roleid, int member_list[], unsigned int cnt)
{
	int index;
	gplayer* pPlayer = world_manager::GetInstance()->FindPlayer(roleid,index);

	if(!pPlayer) return ;
	spin_autolock keeper(pPlayer->spinlock);

	if(pPlayer->ID.id != roleid||!pPlayer->IsActived()||!pPlayer->imp||pPlayer->login_state!=gplayer::LOGIN_OK)
	{
		return;
	}

	gplayer_imp* pImp = (gplayer_imp*)pPlayer->imp;
	pImp->OnAutoTeamComposeStart(member_list, cnt);
}

void notify_serverforbid(std::vector<int> &ctrl_list,std::vector<int> &item_list,std::vector<int> &service_list,std::vector<int> &task_list,std::vector<int> &skill_list, std::vector<int> &shopitem_list, std::vector<int>& recipe_list)
{
	world_manager::GetGlobalController().SetServerForbid(ctrl_list,item_list,service_list,task_list,skill_list, shopitem_list, recipe_list);
}

void notify_servertrigger(std::vector<int> &trigger_list)
{
	world_manager::GetGlobalController().SetServerTrigger(trigger_list);
}

void notify_mafia_pvp_status(int status,std::vector<int> &ctrl_list)
{
	world_manager::GetInstance()->OnMafiaPvPStatusNotice(status,ctrl_list);
}

void request_mafia_pvp_elements(unsigned int version)
{
	world_manager::GetInstance()->OnMafiaPvPElementRequest(version);
}

