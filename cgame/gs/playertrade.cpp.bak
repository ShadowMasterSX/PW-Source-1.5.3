#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>

#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "clstab.h"
#include "actsession.h"
#include "playertemplate.h"
#include "serviceprovider.h"
#include <common/protocol_imp.h>
#include "userlogin.h"
#include "trade.h"
#include <factionlib.h>


int 
gplayer_imp::GeneralTradeMessageHandler(world * pPlane, const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_PICKUP_MONEY:
		case GM_MSG_RECEIVE_MONEY:
		PickupMoneyInTrade(msg.param);
		break;
		case GM_MSG_PICKUP_ITEM:
		PickupItemInTrade(msg.pos,msg.content,msg.content_length,msg.param&0x80000000,msg.param*0x7FFFFFFF);
		break;


		case GM_MSG_SWITCH_GET:
		//不处理
		break;

		case GM_MSG_ENCHANT:
		case GM_MSG_ATTACK:
		case GM_MSG_HURT:
		case GM_MSG_DUEL_HURT:
		case GM_MSG_HATE_YOU:
		case GM_MSG_HEARTBEAT:
		case GM_MSG_QUERY_OBJ_INFO00:
		case GM_MSG_ERROR_MESSAGE:
		case GM_MSG_GROUP_EXPERIENCE:
		case GM_MSG_EXPERIENCE:
		case GM_MSG_TEAM_EXPERIENCE:
//		case GM_MSG_GET_MEMBER_POS:
		case GM_MSG_TEAM_INVITE:
		case GM_MSG_TEAM_AGREE_INVITE:
		case GM_MSG_TEAM_REJECT_INVITE:
		case GM_MSG_JOIN_TEAM:
		case GM_MSG_LEADER_UPDATE_MEMBER:
		case GM_MSG_JOIN_TEAM_FAILED:
		case GM_MSG_MEMBER_NOTIFY_DATA:
		case GM_MSG_NEW_MEMBER:
		case GM_MSG_LEAVE_PARTY_REQUEST:
		case GM_MSG_LEADER_CANCEL_PARTY:
		case GM_MSG_MEMBER_NOT_IN_TEAM:
		case GM_MSG_LEADER_KICK_MEMBER:
		case GM_MSG_MEMBER_LEAVE:
		case GM_MSG_QUERY_PLAYER_EQUIPMENT:
		case GM_MSG_PICKUP_TEAM_MONEY:
		case GM_MSG_NOTIFY_SELECT_TARGET:
		case GM_MSG_QUERY_SELECT_TARGET:

		case GM_MSG_NPC_BE_KILLED:
		case GM_MSG_PLAYER_TASK_TRANSFER:
		case GM_MSG_PLAYER_BECOME_PARIAH:
		case GM_MSG_PLAYER_BECOME_INVADER:
		case GM_MSG_SUBSCIBE_TARGET:
		case GM_MSG_UNSUBSCIBE_TARGET:
		case GM_MSG_SUBSCIBE_CONFIRM:
		case GM_MSG_SUBSCIBE_SUBTARGET:
		case GM_MSG_UNSUBSCIBE_SUBTARGET:
		case GM_MSG_SUBSCIBE_SUBTARGET_CONFIRM:
		case GM_MSG_NOTIFY_SELECT_SUBTARGET:
		case GM_MSG_HP_STEAL:
		case GM_MSG_TEAM_APPLY_PARTY:
		case GM_MSG_TEAM_APPLY_REPLY:
		case GM_MSG_QUERY_INFO_1:
		case GM_MSG_TEAM_CHANGE_TO_LEADER:
		case GM_MSG_TEAM_LEADER_CHANGED:

		case GM_MSG_GM_CHANGE_EXP:
		case GM_MSG_GM_OFFLINE:
		case GM_MSG_GM_MQUERY_MOVE_POS:
		case GM_MSG_GM_DEBUG_COMMAND:
		case GM_MSG_GM_QUERY_SPEC_ITEM:

		case GM_MSG_DBSAVE_ERROR:
		case GM_MSG_QUERY_EQUIP_DETAIL:
		case GM_MSG_ENABLE_PVP_DURATION:
		case GM_MSG_PLAYER_DUEL_REQUEST:
		case GM_MSG_PLAYER_DUEL_REPLY:
		case GM_MSG_PLAYER_DUEL_PREPARE:
		case GM_MSG_PLAYER_DUEL_START:
		case GM_MSG_PLAYER_DUEL_CANCEL:
		case GM_MSG_PLAYER_DUEL_STOP:
		case GM_MSG_PLAYER_BIND_REQUEST:
		case GM_MSG_PLAYER_BIND_INVITE:

		case GM_MSG_PLAYER_RECALL_PET:
		case GM_MSG_MOB_BE_TRAINED:
		case GM_MSG_PET_SET_COOLDOWN:
		case GM_MSG_PET_ANTI_CHEAT:
		case GM_MSG_PET_NOTIFY_DEATH:
		case GM_MSG_PET_NOTIFY_HP:
		case GM_MSG_PET_RELOCATE_POS:
		case GM_MSG_QUERY_PROPERTY:
		case GM_MSG_QUERY_PROPERTY_REPLY:
		case GM_MSG_PLANT_PET_NOTIFY_DEATH:
		case GM_MSG_PLANT_PET_NOTIFY_HP:
		case GM_MSG_PLANT_PET_NOTIFY_DISAPPEAR:
		case GM_MSG_CONGREGATE_REQUEST:
		case GM_MSG_REJECT_CONGREGATE:
		case GM_MSG_NPC_BE_KILLED_BY_OWNER:
		case GM_MSG_EXTERN_HEAL:
		case GM_MSG_QUERY_INVENTORY_DETAIL:
		case GM_MSG_PLAYER_KILLED_BY_PLAYER:

		//这些消息是和普通的时候拥有一样的处理
		return DispatchNormalMessage(pPlane,msg);
	}
	return 0;
}

int 
gplayer_imp::WaitingTradeMessageHandler(world * pPlane ,const MSG & msg)
{
	//只处理部分
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//等待时间超时，发送不能交易消息
			GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),_parent->ID.id,((gplayer*)_parent)->cs_sid,false);
			//回到正常状态
			FromTradeToNormal();
		}
		return DispatchNormalMessage(pPlane,msg);
		break;

		case GM_MSG_ENCHANT:
		if(((enchant_msg*)msg.content)->helpful)
		{
			return GeneralTradeMessageHandler(pPlane,msg);
		}

		//有害法术制止了交易操作
		
		case GM_MSG_HURT:
		case GM_MSG_DUEL_HURT:
		case GM_MSG_ATTACK:
		//发出不能交易消息
		GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),_parent->ID.id,((gplayer*)_parent)->cs_sid,false);
		//回到正常状态
		FromTradeToNormal();
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WaitingTradeCompleteHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//让玩家断线 且不进行存盘
			GLog::log(LOG_ERR,"drop player for trade timeout roleid:%d",_parent->ID.id);
			FromTradeToNormal(-1);
			return 0;
		}
		return DispatchNormalMessage(pPlane,msg);
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WaitingFactionTradeTradeHandler(world * pPlane ,const MSG & msg)
{
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//调用出错处理
			FactionTradeTimeout();
			return 0;
		}
		return DispatchNormalMessage(pPlane,msg);
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::TradeMessageHandler(world * pPlane ,const MSG & msg)
{
	//只处理部分
	switch(msg.message)
	{
		case GM_MSG_ENCHANT:
		if(((enchant_msg*)msg.content)->helpful)
		{
			return GeneralTradeMessageHandler(pPlane,msg);
		}
		//有害法术制止了交易操作

		case GM_MSG_HURT:
		case GM_MSG_DUEL_HURT:
		case GM_MSG_ATTACK:
		//发出不能交易消息
		GMSV::DiscardTrade(_trade_obj->GetTradeID(),_parent->ID.id);
		//回到等待交易完成的状态
		_player_state = PLAYER_WAIT_TRADE_COMPLETE;
		_trade_obj->SetTimeOut(10);

		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WaitingTradeReadHandler(world * pPlane ,const MSG & msg)
{
	//只处理部分
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			//应该让玩家断线，并且不存盘.....
			GLog::log(LOG_ERR,"drop player for trade timeout roleid:%d",_parent->ID.id);
			FromTradeToNormal(-1);
			return 0;
		}
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}

int 
gplayer_imp::WatingFactionTradeReadHandler(world * pPlane ,const MSG & msg)
{
	//只处理部分
	switch(msg.message)
	{
		case GM_MSG_HEARTBEAT:
		if(!_trade_obj->Heartbeat(this))
		{
			GLog::log(LOG_ERR,"drop player for faction trade timeout roleid:%d",_parent->ID.id);

			faction_trade * pTrade = dynamic_cast<faction_trade*>(_trade_obj);
			if(pTrade)
			{
				user_save_data((gplayer*)_parent,NULL,2,pTrade->_put_mask);
			}
			else
			{
				ASSERT(false);
			}

			//应该让玩家断线，并且不存盘.....
			FromFactionTradeToNormal(-1);
			return 0;
		}
		break;
		default:
		return GeneralTradeMessageHandler(pPlane,msg);
	}
	return DispatchNormalMessage(pPlane,msg);
}


bool 
gplayer_imp::CanTrade(const XID & target)
{
	//现在一律禁止交易
//	return false;
	if(_cheat_punish) return false;
	
	//非正常状态不能交易
	if(_player_state != PLAYER_STATE_NORMAL) return false;
	ASSERT(_trade_obj == NULL);

	//交易不能开始的条件有
	//当前有任何类型的session 正在执行中
	if(_cur_session || _session_list.size()) return false;

	if(OI_TestSafeLock()) return false;

/*
	//现在战斗状态允许交易 
	//看看会不会出什么问题..
	//战斗状态也不能交易 
	if(IsCombatState()) return false;
	*/
	
	//死亡状态不能交易 在外面判断
	//还有条件是 距离不能超过范围，在外面判断

	//返回true
	return true;

}

void
gplayer_imp::StartTrade(int trade_id, const XID & target)
{
	//战斗状态现在允许交易
	ASSERT(_player_state == PLAYER_STATE_NORMAL && !_cur_session);
	ASSERT(_trade_obj == NULL);

	//加入交易对象， 进入等待交易状态
	_trade_obj = new player_trade(trade_id);
	_player_state = PLAYER_WAITING_TRADE;
	_trade_obj->SetTimeOut(30);	//设置三十秒存盘超时错误

	class WaitInfoWriteBack : public GDB::Result, public abase::ASmallObject
	{
		int _userid;
		int _cs_index;
		int _cs_sid;
		int _trade_id;
		unsigned int _counter;
		unsigned int _counter2; 
		unsigned int _counter3;
		int _mall_order_id;
		world *_plane;
	public:
		WaitInfoWriteBack(int trade_id, gplayer_imp * imp,world * pPlane):_trade_id(trade_id),_plane(pPlane)
		{
			gplayer * pPlayer = imp->GetParent();
			_userid = pPlayer->ID.id;
			_cs_index = pPlayer->cs_index;
			_cs_sid = pPlayer->cs_sid;
			_counter = imp->_tb_change_counter;
			_counter2 = imp->_eq_change_counter;
			_counter3 = imp->_user_tb_change_counter;
			_mall_order_id = imp->_mall_order_id;
		}

		virtual void OnTimeOut()
		{
			CallWriteBack(false);
			delete this;
		}

		virtual void OnFailed()
		{
			CallWriteBack(false);
			delete this;
		}
		
		virtual void OnPutRole(int retcode)
		{
			ASSERT(retcode == 0);
			CallWriteBack(retcode == 0);
			delete this;
		}

		void CallWriteBack(bool success)
		{
			int index = _plane->FindPlayer(_userid);
			if(index < 0)
			{
				return; //can't find
			}
			gplayer * pPlayer = _plane->GetPlayerByIndex(index);
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _userid || !pPlayer->IsActived()
			  || pPlayer->cs_index != _cs_index || pPlayer->cs_sid != _cs_sid)
			{
				return;	// not match
			}
			ASSERT(pPlayer->imp);
			gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);
			if(success)
			{
				if(_counter == pImp->_tb_change_counter )
				{
					pImp->TryClearTBChangeCounter();
				}

				if(_counter2 == pImp->_eq_change_counter )
				{
					pImp->_eq_change_counter = 0;
				}

				if(_counter3 == pImp->_user_tb_change_counter )
				{
					pImp->TryClearUserTBChangeCounter();
				}

				pImp->MallSaveDone(_mall_order_id);

				//将存盘错误清0
				pImp->_db_save_error = 0;
			}
			pImp->WaitingTradeWriteBack(_trade_id,success);
		}


	};

	//进行存盘操作
	user_save_data((gplayer*)_parent,new WaitInfoWriteBack(trade_id,this,_plane),0);
	return ;
}

void
gplayer_imp::TradeComplete(int trade_id, int reason,bool need_read)
{
	if(_player_state != PLAYER_WAITING_TRADE && 
			_player_state != PLAYER_TRADE &&
			_player_state != PLAYER_WAIT_TRADE_COMPLETE)
	{
		//未在合适的状态中
		return;
	}

	class WaitInfoReadBack: public GDB::Result, public abase::ASmallObject
	{
		int _userid;
		int _cs_index;
		int _cs_sid;
		int _trade_id;
		world * _plane;
	public:
		WaitInfoReadBack(int trade_id, gplayer * pPlayer,world *pPlane):_trade_id(trade_id),_plane(pPlane)
		{
			_userid = pPlayer->ID.id;
			_cs_index = pPlayer->cs_index;
			_cs_sid = pPlayer->cs_sid;
		}

		virtual void OnTimeOut()
		{
			CallReadBack(NULL);
			delete this;
		}

		virtual void OnFailed()
		{
			CallReadBack(NULL);
			delete this;
		}
		
		virtual void OnGetMoneyInventory(size_t money, const GDB::itemlist & list, int timestamp,int mask)
		{
			CallReadBack(&list, money);
			delete this;
		}

		void CallReadBack(const GDB::itemlist * pInv,int money = 0)
		{
			int index = _plane->FindPlayer(_userid);
			if(index < 0)
			{
				return; 
			}
			gplayer * pPlayer = _plane->GetPlayerByIndex(index);
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _userid || !pPlayer->IsActived()
			  || pPlayer->cs_index != _cs_index || pPlayer->cs_sid != _cs_sid)
			{
				return;
			}
			ASSERT(pPlayer->imp);
			((gplayer_imp*)pPlayer->imp)->WaitingTradeReadBack(_trade_id,pInv,money);
		}


	};

	if(_trade_obj->GetTradeID() != trade_id)
	{
		//交易ID不符合则断线之
		GLog::log(GLOG_ERR,"用户%d交易ID%d完成后ID不符合(应为%d)",_parent->ID.id, trade_id,_trade_obj->GetTradeID());
		FromTradeToNormal(-1);
		return;
	}

	if(need_read)
	{	
		//进入读盘等待状态
		_player_state = PLAYER_WAIT_TRADE_READ;
		//发起读盘请求并设置超时
		_trade_obj->SetTimeOut(45);

		GDB::get_money_inventory(_parent->ID.id,new WaitInfoReadBack(trade_id,(gplayer*)_parent,_plane));

	}
	else
	{
		//不需要重新读盘，直接返回normal状态
		FromTradeToNormal();
	}
	return;
}

bool
gplayer_imp::StartFactionTrade(int trade_id,int get_mask, int put_mask ,bool no_response)
{
	if(_player_state != PLAYER_STATE_NORMAL || _cur_session) return false;
	ASSERT(_trade_obj == NULL);

	//加入交易对象， 进入等待交易状态
	_trade_obj = new faction_trade(trade_id, get_mask,put_mask);
	_player_state = PLAYER_WAITING_FACTION_TRADE;
	_trade_obj->SetTimeOut(45);	//设置三十秒超时错误

	if(!no_response)
	{
		//发送个人数据
		//no_response=false只在gamed向gfactiond发送FactionOPRequest时出现，获取snycdata时应增加_db_timestamp
		int timestamp = OI_InceaseDBTimeStamp();
		GLog::log(GLOG_INFO,"用户%d开始帮派交易,timestamp=%d,money=%d", _parent->ID.id, timestamp, GetMoney());
		GNET::syncdata_t data(GetMoney(),_basic.skill_point);
		GNET::SendFactionLockResponse(0,trade_id,_parent->ID.id,data);
	}
	((gplayer_dispatcher*)_runner)->mafia_trade_start();
	return true;
}
void 
gplayer_imp::WaitingTradeWriteBack(int trade_id,bool bSuccess)
{
	if(_player_state != PLAYER_WAITING_TRADE) return;
	if(_trade_obj->GetTradeID() != trade_id) return;

	gplayer * pPlayer = (gplayer*)_parent;
	if(bSuccess)
	{
		//清除一下仓库改变记录
		TryClearTBChangeCounter();

		//写入数据成功
		//发送同意交易的数据
		GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),pPlayer->ID.id,pPlayer->cs_sid,true);

		//进入交易状态
		_player_state = PLAYER_TRADE;
	}
	else
	{
		//写入数据失败
		//发送不同意交易的数据
		GMSV::ReplyTradeRequest(_trade_obj->GetTradeID(),pPlayer->ID.id,pPlayer->cs_sid,false);

		//回到非交易状态
		FromTradeToNormal();
	}
}

void 
gplayer_imp::WaitingTradeReadBack(int trade_id,const GDB::itemlist * pInv,int money)
{
	if(_player_state != PLAYER_WAIT_TRADE_READ) return;
	if(_trade_obj->GetTradeID() != trade_id) return;
	if(pInv)
	{
		GLog::log(GLOG_INFO,"用户%d交易成功，交易ID%d金钱改变为%d",_parent->ID.id, trade_id,money - GetMoney());
		//读取数据成功
		//重新整理自己的物品栏
		_inventory.Clear();
		_inventory.InitFromDBData(*pInv);
		if(((size_t)money) > _money_capacity) 
		{
			//
			GLog::log(GLOG_ERR,"用户%d交易金钱超过最大容量",_parent->ID.id);
			money = _money_capacity;
		}
		_player_money = money;

		//回到通常状态
		FromTradeToNormal();
	}
	else
	{
		GLog::log(GLOG_INFO,"用户%d交易ID%d完成后取数据失败",_parent->ID.id, trade_id);
		//读取数据失败 直接下线是个好主意
		//并且不进行写盘操作
		FromTradeToNormal(-1);
	}
}

void 
gplayer_imp::PutItemAfterTrade(item_list & backpack)
{
	XID self = _parent->ID;
	for(size_t i = 0; i < backpack.Size();i ++)
	{
		if(backpack[i].type == -1) break;
		item it;
		backpack.Remove(i,it);
		ASSERT(it.type != -1);
		size_t ocount = it.count;
		int expire_date = it.expire_date;
		int type = it.type;
		int rst = _inventory.Push(it);
		if(rst >= 0)
		{
			//获得了东西
			((gplayer_dispatcher*)_runner)->receive_item_after_trade(type,expire_date,ocount-it.count,_inventory[rst].count,rst);
		}

		if(it.type != -1)
		{	
			//无法放下了，进行丢弃的操作
			item_data data;
			item_list::ItemToData(it,data);
			DropItemFromData(_plane,_parent->pos,data,self,0,0,_parent->ID.id); 
			it.Release();
		}
	}
}

void 
gplayer_imp::DropAllAfterTrade(item_list & backpack,size_t money)
{
	XID self = _parent->ID;
	const A3DVECTOR & pos = _parent->pos;
	for(size_t i = 0; i < backpack.Size();i ++)
	{
		if(backpack[i].type == -1) break;
		item it;
		backpack.Remove(i,it);
		ASSERT(it.type != -1);
		item_data data;
		item_list::ItemToData(it,data);
		DropItemFromData(_plane,pos,data,self,0,0,_parent->ID.id); 
		it.Release();
	}
	if(money) DropMoneyItem(_plane,pos,money,self,0,0);
}

void 
gplayer_imp::FromTradeToNormal(int type)
{
	if(type < 0)
	{
		//将物品扔在地上
		DropAllAfterTrade(_trade_obj->GetBackpack(),_trade_obj->GetMoney());

		delete _trade_obj;
		_trade_obj = NULL;
		_player_state = PLAYER_STATE_NORMAL;

		//然后进行下线操作，不进行存盘
		_offline_type = PLAYER_OFF_LPG_DISCONNECT;
		Logout(-1);
		return;
	}
	//告诉玩家当前的包裹栏内容
	PlayerGetInventoryDetail(0);
	_runner->get_player_money(GetMoney(),_money_capacity);

	//首先将积累的物品和钱交给玩家
	size_t inc_money = _trade_obj->GetMoney();
	if(inc_money)
	{
		size_t tmp = GetMoney();
		GainMoneyWithDrop(inc_money);
		((gplayer_dispatcher*)_runner)->receive_money_after_trade(GetMoney() - tmp);
	}
	PutItemAfterTrade(_trade_obj->GetBackpack());

	//然后删除对象，回到交易状态
	delete _trade_obj;
	_trade_obj = NULL;

	//然后回到通用状态 并且通知player现在处于非交易状态
	//死亡状态已经独立出来了
	_player_state = PLAYER_STATE_NORMAL; 
	_write_timer = 512;

	if(_parent->b_disconnect)
	{
		//玩家已经断线，执行断线逻辑  并且存盘
		//进入断线逻辑
		_player_state = PLAYER_DISCONNECT;
		_disconnect_timeout = LOGOUT_TIME_IN_NORMAL;
	}
}

void 
gplayer_imp::FromFactionTradeToNormal(int type)
{
	if(type)
	{
		//将物品扔在地上
		DropAllAfterTrade(_trade_obj->GetBackpack(),_trade_obj->GetMoney());

		delete _trade_obj;
		_trade_obj = NULL;
		_player_state = PLAYER_STATE_NORMAL;

		//然后进行下线操作，不进行存盘
		_offline_type = PLAYER_OFF_LPG_DISCONNECT;
		Logout(-1);
		return;
	}

	ASSERT(_player_state == PLAYER_WAITING_FACTION_TRADE || _player_state == PLAYER_WAIT_FACTION_TRADE_READ);
	//告诉玩家当前的包裹栏内容
	//判断是否告诉玩家刷新装备 尚未做
	//PlayerGetInventory(0);
	_runner->get_player_money(GetMoney(),_money_capacity);

	//首先将积累的物品和钱交给玩家
	size_t inc_money = _trade_obj->GetMoney();
	if(inc_money)
	{
		size_t tmp = GetMoney();
		GainMoneyWithDrop(inc_money);
		((gplayer_dispatcher*)_runner)->receive_money_after_trade(GetMoney() - tmp);
	}
	PutItemAfterTrade(_trade_obj->GetBackpack());

	//帮派交易现在先不重新发送物品数据
	//PlayerGetInventory(0);

	//然后删除对象
	delete _trade_obj;
	_trade_obj = NULL;

	//然后回到通用状态 并且通知player现在处于非交易状态
	//死亡状态已经独立出来了
	_player_state = PLAYER_STATE_NORMAL; 

	((gplayer_dispatcher*)_runner)->mafia_trade_end();
	if(_parent->b_disconnect)
	{
		//玩家已经断线，执行断线逻辑  并且存盘
		Logout(GMSV::PLAYER_LOGOUT_FULL);
	}
}

void 
gplayer_imp::FactionTradeTimeout()
{
	ASSERT(_player_state == PLAYER_WAITING_FACTION_TRADE);
	class WaitInfoReadBack: public GDB::Result, public abase::ASmallObject
	{
		int _userid;
		int _cs_index;
		int _cs_sid;
		world * _plane;
	public:
		WaitInfoReadBack(gplayer * pPlayer,world *pPlane):_plane(pPlane)
		{
			_userid = pPlayer->ID.id;
			_cs_index = pPlayer->cs_index;
			_cs_sid = pPlayer->cs_sid;
		}

		virtual void OnTimeOut()
		{
			//CallReadBack(NULL);
			//这里不能在调用rpc了，会死锁
			//所以使用trade_obj的超时
			delete this;
		}

		virtual void OnFailed()
		{
			CallReadBack(NULL);
			delete this;
		}
		
		virtual void OnGetMoneyInventory(size_t money, const GDB::itemlist & list,int timestamp, int mask)
		{
			CallReadBack(&list, money, timestamp, mask);
			delete this;
		}

		void CallReadBack(const GDB::itemlist * pInv,int money = 0, int timestamp = 0, int mask = 0)
		{
			int index = _plane->FindPlayer(_userid);
			if(index < 0)
			{
				return; 
			}
			gplayer * pPlayer = _plane->GetPlayerByIndex(index);
			spin_autolock keeper(pPlayer->spinlock);

			if(pPlayer->ID.id != _userid || !pPlayer->IsActived()
			  || pPlayer->cs_index != _cs_index || pPlayer->cs_sid != _cs_sid)
			{
				return;	
			}
			ASSERT(pPlayer->imp);
			gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);
			pImp->WaitingFactionTradeReadBack(pInv, money, timestamp, mask);
		}

	};

	//进入读盘等待状态
	_player_state = PLAYER_WAIT_FACTION_TRADE_READ;
	//发起读盘请求并设置超时
	_trade_obj->SetTimeOut(45);

	faction_trade * pTrade = dynamic_cast<faction_trade*>(_trade_obj);
	if(pTrade)
	{
		GDB::get_money_inventory(_parent->ID.id,new WaitInfoReadBack((gplayer*)_parent,_plane),pTrade->_get_mask);
	}
	else
	{
		ASSERT(false);
		//应该让玩家断线，并且不存盘.....
		FromFactionTradeToNormal(-1);
	}
	return;
}

void 
gplayer_imp::WaitingFactionTradeReadBack(const GDB::itemlist * pInv,int money, int timestamp, int mask)
{
	if(_player_state != PLAYER_WAIT_FACTION_TRADE_READ) return;
	if(pInv)
	{
		if(((int)(timestamp -_db_timestamp)) >= 0)
		{       
			//修正钱数
			if(((size_t)money) > _money_capacity) 
			{       
				//      
				GLog::log(GLOG_ERR,"用户%d外部交易超时后，金钱超过最大容量",_parent->ID.id);
				money = _money_capacity;
			}       
			_player_money = money;
			GLog::log(GLOG_INFO,"用户%d外部交易超时后，金钱改变 %d" ,_parent->ID.id,money - GetMoney());

			//读取数据成功
			//重新整理自己的物品栏
			_inventory.Clear();
			_inventory.InitFromDBData(*pInv);
			PlayerGetInventoryDetail(0);
			_db_timestamp = timestamp;
		}       
		else    
		{       
			GLog::log(GLOG_ERR,"用户%d外部交易结束，但时戳较小%d(%d)",_parent->ID.id,timestamp,_db_timestamp);
			//读出来的时间戳过小，不使用 而是用自己的数据
		}               

		//回到通常状态
		FromFactionTradeToNormal();
	}
	else
	{
		//读取数据失败 
		GLog::log(GLOG_ERR,"用户%d外部交易完成后取数据失败",_parent->ID.id);

		//保存一次非包裹数据
		faction_trade * pTrade = dynamic_cast<faction_trade*>(_trade_obj);
		if(pTrade)
		{       
			user_save_data((gplayer*)_parent,NULL,2,pTrade->_put_mask);
		}
		else
		{
			ASSERT(false);
		}

		//并且不进行写盘操作
		FromFactionTradeToNormal(-1);
	}
}

void 
gplayer_imp::FactionTradeComplete(int trade_id,const GNET::syncdata_t & data)
{
	if(_player_state != PLAYER_WAITING_FACTION_TRADE)
	{
		//未在合适的状态中
		return;
	}
	GLog::log(GLOG_INFO,"用户%d帮派操作完成，金钱改变为%d",_parent->ID.id, data.money - GetMoney());

	//更新数据
	ASSERT(_trade_obj != NULL);
	ASSERT(data.money >=0 && data.money <= _money_capacity);
	
	if(trade_id != _trade_obj->GetTradeID()) return ;

	//发送成功数据
	//通知客户端完成此操作，可以进行操作
	_player_money = data.money;
	_basic.skill_point += data.sp;
	if(data.sp)
	{
		GLog::log(GLOG_INFO,"用户%d进行帮派操作(tid:%d)，sp改变%d",_parent->ID.id,trade_id,data.sp);
	}
	
	SetRefreshState();
	//GSMV_SEND FACTIONTRADECOMPELETE
	//最好进行存盘操作
	
	FromFactionTradeToNormal();
	return;
}

void 
gplayer_imp::SyncTradeComplete(int trade_id,size_t money, const GDB::itemlist & item_change,bool writetrashbox, bool cash_change)
{
	if(_player_state != PLAYER_WAITING_FACTION_TRADE)
	{
		//未在合适的状态中
		return;
	}
	//更新数据
	ASSERT(_trade_obj != NULL);
	ASSERT(money >=0 && money <= _money_capacity);
	
	GLog::log(GLOG_INFO,"用户%d外部操作完成，金钱改变为%d",_parent->ID.id, money - GetMoney());

	if(trade_id != _trade_obj->GetTradeID()) return ;

	//发送成功数据
	//通知客户端完成此操作，可以进行操作
	_player_money = money;

	//进行包裹栏的修改
	for(size_t i = 0; i < item_change.count; i ++)
	{
		GDB::itemdata * pData = item_change.list + i;
		if(pData->count)
		{
			item & it = _inventory[pData->index];
			//更新操作
			if(pData->id != (unsigned int)it.type) 
			{
				_inventory.Remove(pData->index);
				item it2;
				if(MakeItemEntry(it2,*pData))
				{
					bool bRst = _inventory.Put(pData->index,it2);
					ASSERT(bRst);
				}
				else
				{
					GLog::log(LOG_ERR, "SyncTradeComplete时创建物品失败%d",it2.type);
					it2.Clear();
				}
			}
			else
			{
				if((unsigned int)pData->count > it.count)
				{
					_inventory.IncAmount(pData->index, pData->count - it.count);
				}
				else
				{
					_inventory.DecAmount(pData->index, it.count - pData->count);
				}
			}
		}
		else
		{
			//删除操作
			_inventory.Remove(pData->index);
		}
	}
	
/*	if(writetrashbox) $$$$$$$$$$$ 现在一律不清楚仓库存盘标志
	{
		TryClearTBChangeCounter();
	}
	*/
	
	PlayerGetInventoryDetail(0);

	if(cash_change)
	{
		MallSaveDone(_mall_order_id);
	}

	FromFactionTradeToNormal();
	return;
}

