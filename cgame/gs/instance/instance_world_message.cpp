#include "instance_manager.h"
#include "../world.h"
#include "../player_imp.h"
#include "../usermsg.h"

/*
 *	处理世界消息
 */
int
instance_world_message_handler::RecvExternMessage(int msg_tag, const MSG & msg)
{
	//对于副本，只接受给万家的消息
	if(msg.target.type != GM_TYPE_PLAYER && msg.target.type != GM_TYPE_SERVER ) return 0;
	if(msg_tag != world_manager::GetWorldTag())
	{
		//隔断某些消息.......
	}

	//还需要要隔断某些消息
	//这里还应该直接处理某些消息
	//数据转发需要经过判定

	//有些消息需要经过特殊转发
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			{
			//这里查询KEY的方式不是很正确， 有待考虑 $$$$$$$$$$
				if(msg.content_length < sizeof(instance_key)) return 0;
				instance_key * key = (instance_key*)msg.content;
				//消息的头部必须是instance_key
				//ASSERT(key->target.key_level1 == msg.source.id);
				//这里不限制了，主要是GM会有此行为
				instance_hash_key hkey;
				_manager->TransformInstanceKey(key->target, hkey);
				int index = _manager->GetWorldByKey(hkey);
				if(index < 0) return 0;
				return _manager->GetWorldByIndex(index)->DispatchMessage(msg);
			}

		case GM_MSG_PLANE_SWITCH_REQUEST:
		//确定切换请求 
		//直接返回反馈 可能需要检查是否可以建立副本世界
		{
			//首先检查是否可以建立世界， 由manager决定，其次检查世界的玩家人数是否达到上限
			if(msg.content_length != sizeof(instance_key)) 
			{
				ASSERT(false);
				return 0;
			}
			instance_key * key = (instance_key*)msg.content;
			int rst;
			if((rst = _manager->CheckPlayerSwitchRequest(msg.source,key,msg.pos,msg.param)) == 0)
			{
				//进行回馈消息
				MSG nmsg = msg;
				nmsg.target = msg.source;
				nmsg.source = msg.target;
				nmsg.message = GM_MSG_PLANE_SWITCH_REPLY;
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
			else
			{
				MSG nmsg;
				BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,msg.source,msg.target,msg.pos,rst);
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
		}
		return 0;

		default:
		if(msg.target.type == GM_TYPE_PLAYER)
		{
			int index = _manager->GetPlayerWorldIdx(msg.target.id);
			if(index < 0) return 0;
			return _manager->GetWorldByIndex(index)->DispatchMessage(msg);
		}
		//服务器消息尚未处理 ..........
	}
	return 0;
}


/**临时处理用户从其他服务器转移*/
int 
instance_world_message_handler::PlayerComeIn(instance_world_manager *pManager, world * pPlane,const MSG &msg)
{
	gplayer * pPlayer = pPlane->GetPlayerByID(msg.source.id);
	if(!pPlayer)
	{
		//这次转移已经超时
		__PRINTF("用户移入,但是用户已经超时\n");
		GLog::log(GLOG_ERR, "Time out when player come in. worldtag:%d:roleid:%d", world_manager::GetWorldTag(), msg.source.id);
		return 0;
	}
	spin_autolock keeper(pPlayer->spinlock);
	if(pPlayer->ID != msg.source || !pPlayer->IsActived() || pPlayer->login_state != gplayer::WAITING_SWITCH)
	{
		//player的状态不正确
		__PRINTF("用户移入,但是用户对象的状态不正确襖n");
		GLog::log(GLOG_ERR, "Invalid state when player come in. worldtag:%d:roleid:%d", world_manager::GetWorldTag(), msg.source.id);
		return 0;
	}
	ASSERT(pPlayer->imp == NULL);
	g_timer.remove_timer(pPlayer->base_info.faction,(void*)pPlayer->base_info.race);

	//删除超时timer

	raw_wrapper wrapper(msg.content,msg.content_length);
	wrapper.SetLimit(raw_wrapper::LOAD_ONLY);
	instance_key ikey;
	int source_tag;
	wrapper >> ikey >> source_tag;
	if(ikey.target.key_level1 != msg.param)
	{
		//key的基础判断不满足
		GLog::log(GLOG_ERR,"invalid instance key while switch %d",pPlayer->ID.id);
		pPlane->FreePlayer(pPlayer);
		//恢复对象失败或者插入失败
		ASSERT(false);
		return 0;
	}

	pPlayer->Import(wrapper);
	ASSERT(msg.source == pPlayer->ID);

	int rst = RestoreObject(wrapper,pPlayer,pPlane);
	//设置新的坐标 还需要保存旧的坐标用来存盘
	A3DVECTOR source_pos = pPlayer->pos;

	pManager->SetIncomingPlayerPos(pPlayer,msg.pos,ikey.special_mask);
	
	//检查对象的存在关系
	if(rst < 0 || pPlane->InsertPlayer(pPlayer) <0)
	{       
		pPlane->FreePlayer(pPlayer);
		//恢复对象失败或者插入失败
		//需要做日志并清除NPC对象
		//还要进行数据清除 操作，比如PlayerMap,manager的player world对应关系等等
		ASSERT(false);
		return 0;
	} 
	pPlane->InsertPlayerToMan(pPlayer);
	
	//试图更新该对象的类
	world_manager::player_cid  cid;
	pManager->GetPlayerCid(cid);
	TrySwapPlayerData(pPlane,cid.cid,pPlayer);

	gplayer_imp * pImp = ((gplayer_imp*)pPlayer->imp);
	PlayerPreEnterServer(pPlayer, pImp,ikey);

	pImp->PlayerEnterServer();

	//设定存盘的时刻， 加快了存盘的速度 现在是提前存盘了
//	pImp->_write_timer = abase::Rand(3, 30);
	if(world_manager::GetWorldLimit().clearap)
	{
		pImp->ModifyAP(-100000);
	}

	//发送转移成功的命令
	GMSV::SendSwitchServerSuccess(pPlayer->cs_index,pPlayer->ID.id, pPlayer->cs_sid,world_manager::GetWorldIndex());

	pManager->PlayerAfterSwitch(pImp);
	//设置副本的检测filter
	pManager->SetFilterWhenLogin(pImp,&ikey);

	//设置进入副本的一些数据
	pImp->SetInstanceTimer(world_manager::GetWorldTag());
	pImp->SetLastInstancePos(world_manager::GetWorldTag(),pPlayer->pos, pPlane->w_create_timestamp);
	pImp->SetLastInstanceSourcePos(source_tag, source_pos);

	//将自己的位置信息发送一下
	dispatcher * runner = pPlayer->imp->_runner;
	runner->notify_pos(pPlayer->pos);
	//重新发送玩家数据
	runner->begin_transfer();
	runner->enter_world();
	runner->end_transfer();
	runner->server_config_data();

	GLog::log(GLOG_INFO,"用户%d(%d,%d)转移到%d",pPlayer->ID.id, pPlayer->cs_index,pPlayer->cs_sid,world_manager::GetWorldTag());

	timeval tv;
	gettimeofday(&tv,NULL);
	__PRINTF("%d转移服务器完成:%u.%u\n",pPlayer->ID.id,tv.tv_sec,tv.tv_usec);
	return 0;
}

int
instance_world_message_handler::HandleMessage(world * pPlane,const MSG & msg)
{
	//有些消息操作可能会比较费时间，是否可以考虑Task完成，不过用线程的话就要考虑msg的数据问题了。
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			return PlayerComeIn(_manager,pPlane,msg);

		default:
			world_message_handler::HandleMessage(pPlane,msg);
	}
	return 0;
}

