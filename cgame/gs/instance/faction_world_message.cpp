#include "instance_manager.h"
#include "../world.h"
#include "../player_imp.h"
#include "../usermsg.h"
#include "faction_world_ctrl.h"

/*
 *	处理世界消息
 */
int
faction_world_message_handler::RecvExternMessage(int msg_tag, const MSG & msg)
{
	//对于副本，只接受给万家的消息
	if(msg.target.type != GM_TYPE_PLAYER && msg.target.type != GM_TYPE_SERVER ) return 0;
	if(msg_tag != world_manager::GetWorldTag())
	{
		//隔断某些消息.......
	}

	//还需要要隔断某些消息
	//$$$$$$这里还应该直接处理某些消息
	//$$$$$$$$数据转发需要经过判定

	//有些消息需要经过特殊转发
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			{
				if(msg.content_length < sizeof(instance_key)) return 0;
				instance_key * key = (instance_key*)msg.content;
				//消息的头部必须是instance_key
				//后面附加着玩家数据
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
		//检查副本世界是否存在，如果不存在，则放入等待列表?		//如果已经存在，则刷新一下服务器的时间标志，并返回成功标志
		//这个操作要进行锁定，以免删除世界 
		{
			if(msg.content_length != sizeof(instance_key)) 
			{
				ASSERT(false);
				return 0;
			}
			instance_key * key = (instance_key*)msg.content;
			int rst = _manager->CheckPlayerSwitchRequest(msg.source,key,msg.pos,msg.param);
			if(rst == 0)
			{
				//进行回馈消息
				MSG nmsg = msg;
				nmsg.target = msg.source;
				nmsg.source = msg.target;
				nmsg.message = GM_MSG_PLANE_SWITCH_REPLY;
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
			else if(rst > 0)
			{
				MSG nmsg;
				BuildMessage(nmsg,GM_MSG_ERROR_MESSAGE,msg.source,msg.target,msg.pos,rst);
				_manager->SendRemotePlayerMsg(msg.source.id, nmsg);
			}
			//如果rst小于0，表明当前无法确定是否能够创建世界，需要等待 所以反而什么都不作
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

int
faction_world_message_handler::HandleMessage(world * pPlane,const MSG & msg)
{
	//有些消息操作可能会比较费时间，是否可以考虑Task完成，不过用线程的话就要考虑msg的数据问题了。

	//考虑将一些内容移动到基类中 $$$$$$
	switch(msg.message)
	{
		case GM_MSG_SWITCH_USER_DATA:
			return PlayerComeIn(_manager,pPlane,msg);

		default:
			world_message_handler::HandleMessage(pPlane,msg);
	}
	return 0;
}

void 
faction_world_message_handler::PlayerPreEnterServer(gplayer * pPlayer, gplayer_imp * pImp,instance_key &  ikey)
{
	world * pPlane = pImp->_plane;
	//根据玩家的帮派设定攻方和守方
	//增加人数的操作在 玩家的EnterServer操作中完成
	faction_world_ctrl * pCtrl = (faction_world_ctrl *)pPlane->w_ctrl;

	if(pPlayer->id_mafia == pCtrl->factionid)
	{
		//守方
		pPlayer->SetBattleDefence();
		//检查人数上限 注意减少人数上限是在player_battle里的PlayerLeaveWorld里做的
		if(!pCtrl->AddDefender())
		{
			//人数已满,清除里面的帮派内容
			ikey.target.key_level3 = -1;

			//清除战场的标志(后面有用)
			pPlayer->ClrBattleMode();
		}
	}
	else if(pCtrl->inbattle && pPlayer->id_mafia == pCtrl->offense_faction)
	{
		//攻方
		pPlayer->SetBattleOffense();
		//检查人数上限 注意减少人数上限是在player_battle里的PlayerLeaveWorld里做的
		if(!pCtrl->AddAttacker())
		{
			//人数已满,清除里面的帮派内容
			ikey.target.key_level3 = -1;

			//清除战场的标志(后面有用)
			pPlayer->ClrBattleMode();
		}
	}
}
