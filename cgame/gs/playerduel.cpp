#include "playerduel.h"
#include "world.h"
#include "player_imp.h"
#include "obj_interface.h"
#include "duel_filter.h"
#include "filter_man.h"
#include "sfilterdef.h"

void 
player_duel::PlayerDuelRequest(gplayer_imp * pImp,const XID & target)
{
	//调用前外面进行了是否战斗模式和其他模式的判断
	if(_invite.is_invite || _invite.is_agree_duel)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_DUEL_TWICE);
		return ;
	}

	if(!target.IsPlayer())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_TARGET);
		return;
	}
	
	if(IsDuelMode())
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_DUEL_TWICE);
		return ;
	}
	
	_invite.duel_target = target;
	_invite.timeout = DUEL_INVITE_TIMEOUT;
	_invite.is_invite = true;
	_invite.is_agree_duel = false;
	pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_REQUEST,target,0);
	
}

void
player_duel::PlayerDuelReply(gplayer_imp * pImp,const XID & target,int param)
{
	if(!target.IsPlayer())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_TARGET);
		return;
	}

	if(_invite.is_agree_duel || param )
	{
		//如果已经同意决斗 或者拒绝了决斗，则忽略此次决斗消息
		//拒绝了决斗
		pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_REPLY,target,param);
		return;
	}
	
	if(IsDuelMode())
	{
		//如果已经开始决斗， 则拒绝决斗
		pImp->_runner->error_message(S2C::ERR_CANNOT_DUEL_TWICE);
		return ;
	}
	
	_invite.duel_target = target;
	_invite.timeout = 3;	//这个就几秒超时了
	_invite.is_invite = false;
	_invite.is_agree_duel = true;
	
	//同意了决斗,发出同意的消息
	pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_REPLY,target,param);
}

void 
player_duel::MsgDuelRequest(gplayer_imp * pImp,const XID & target)
{
	__PRINTF("MsgDuelRequest\n");
	if(_invite.is_agree_duel || IsDuelMode() || pImp->IsCombatState() || ((gplayer*)pImp->_parent)->IsInvisible())
	{
		//直接拒绝决斗
		pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_REPLY,target,DUEL_REPLY_ERR_STATUS);
		return;
	}
	
	pImp->_runner->duel_recv_request(target);
}

void 
player_duel::MsgDuelReply(gplayer_imp * pImp,const XID & who,int param)
{
	__PRINTF("MsgDuelReply\n");
	if(IsDuelMode())
	{
		//如果已经进入决斗，则忽略
		return;
	}
	
	if(param)
	{
		//请求被拒绝
		if(_invite.is_invite && _invite.duel_target == who)
		{
			_invite.is_invite = false;
			pImp->_runner->duel_reject_request(who,param);
		}
		else if(_invite.is_agree_duel && _invite.duel_target == who)
		{
			_invite.is_agree_duel = false;
			pImp->_runner->error_message(S2C::ERR_CREATE_DUEL_FAILED);
		}
		return ;
	}
	
	if(!_invite.is_invite || _invite.duel_target != who || pImp->IsCombatState())
	{
		//玩家不符合决斗要求 返回一个请求失败的情况
		pImp->SendTo<0>(GM_MSG_ERROR_MESSAGE,who,S2C::ERR_CREATE_DUEL_FAILED);
		return ;
	}

	//可以开始决斗
	//进入决斗倒计时状态,并通知双方
	_invite.is_agree_duel = false;
	_invite.is_invite = false;
	_duel_target = who;
	SetDuelPrepareMode();
	_duel_timeout = 3;	//3秒预备

	//通知对方开始
	pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_PREPARE,who,0);
	pImp->_runner->duel_prepare(who,3);
}

void 
player_duel::MsgDuelPrepare(gplayer_imp * pImp,const XID & who)
{
	__PRINTF("MsgDuelPrepare\n");
	if(_invite.is_agree_duel && who == _invite.duel_target)
	{
		_invite.is_agree_duel = false;
		_invite.is_invite = false;
		_duel_target = who;
		SetDuelPrepareMode();
		_duel_timeout = 10;	//10秒预备,其原因是等待另外一个人发出开始决斗的消息

		pImp->_runner->duel_prepare(who,3);
	}
	else
	{
		//通知对象停止决斗?
		pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_CANCEL,who,0);
	}
}

void 
player_duel::MsgDuelCancel(gplayer_imp * pImp, const XID & who)
{
	__PRINTF("MsgDuelCancel\n");
	if(!IsDuelMode() || _duel_target != who) return;
	ClearDuelMode(pImp);
	_duel_target = XID(-1,-1);
	pImp->_runner->duel_cancel(who);
}

void 
player_duel::MsgDuelStart(gplayer_imp * pImp,const XID & who)
{
	__PRINTF("MsgDuelStart\n");
	if(!(IsPrepareMode() && _duel_target == who))
	{
		//状态不正确，返回一个Cancel
		pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_CANCEL,who,0);
		return;
	}
	SetDuelStartMode(pImp);
}

void
player_duel::MsgDuelStop(gplayer_imp * pImp, const XID & who,int param)
{
	__PRINTF("MsgDuelStop\n");
	if(!IsDuelMode() || _duel_target != who) return;
	ClearDuelMode(pImp);
	_duel_target = XID(-1,-1);
}

void 
player_duel::Heartbeat(gplayer_imp *pImp)
{
	if(IsDuelMode())
	{
		if(IsPrepareMode())
		{	
			__PRINTF("DuelPrepare Timer\n");
			//这时预备决斗模式
			_duel_timeout --;
			if(_duel_timeout <= 0)
			{
				//发送决斗开始的消息
				_duel_timeout = 3;
				XID list[2] =  {pImp->_parent->ID,_duel_target};
				MSG msg;
				BuildMessage(msg,GM_MSG_PLAYER_DUEL_START,list[1],
						list[0],pImp->_parent->pos, 0, &_duel_target, sizeof(_duel_target));
				pImp->_plane->SendMessage(list,list + 2, msg);
			}
		}
		else
		{
			__PRINTF("Duel Timer\n");
			//正在决斗的模式
			_duel_timeout --;
			bool bRst = true;
			int val = 0;
			if(_duel_timeout <= 0)
			{
				bRst = false;
				val = DUEL_RESULT_TIMEOUT;
			}
			if(bRst)
			{
				//没有超时，进行范围和位置的判断
				world::object_info info;
				if(!pImp->_plane->QueryObject(_duel_target,info)
						|| (info.state & world::QUERY_OBJECT_STATE_ZOMBIE)
						|| (info.pos.squared_distance(pImp->_parent->pos)) >= 100.f*100.f)
				{
					val = DUEL_RESULT_DEUCE;
					bRst = false;
				}
			}

			if(!bRst)
			{
				//超时,决斗结束
				//发送结束决斗的消息
				_duel_timeout = 3;
				XID list[2] =  {pImp->_parent->ID,_duel_target};
				MSG msg;
				BuildMessage(msg,GM_MSG_PLAYER_DUEL_STOP,list[1],
						list[0],pImp->_parent->pos,
						val,&_duel_target,sizeof(_duel_target));
				pImp->_plane->SendMessage(list,list + 2, msg);
				pImp->_runner->duel_result(_duel_target, false);
				
				time_t t = time(NULL);
				char buf[32];
				strftime(buf, sizeof(buf), "%F %T", localtime(&t)); 
				GLog::log(GLOG_INFO, "duel:time(%s),player1(%d),player2(%d),winner(0)", buf, pImp->_parent->ID.id, _duel_target.id);
			}
		}
	}
	else
	{
		if(_invite.is_invite)
		{
			__PRINTF("Duel Invite Timer\n");
			_invite.timeout --;
			if(_invite.timeout <=0) 
			{
				//邀请失败
				pImp->_runner->duel_reject_request(_invite.duel_target,DUEL_REPLY_TIMEOUT);
				_invite.is_invite = false;
			}
		}
		else
		if(_invite.is_agree_duel)
		{
			__PRINTF("Duel Agree Timer\n");
			_invite.timeout --;
			if(_invite.timeout <=0) 
			{
				//邀请失败
				pImp->_runner->error_message(S2C::ERR_CREATE_DUEL_FAILED);
				_invite.is_agree_duel = false;
			}
		}
	}
}

void 
player_duel::OnDeath(gplayer_imp *pImp, bool duel_failed)
{
	if(duel_failed)
	{
		__PRINTF("Duel OnDeath \n");
		//决斗失败
		if(IsDuelStarted())
		{
			//发送消息
			pImp->_runner->duel_result(_duel_target, true);

			//发出决斗结束的消息
			pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_STOP,_duel_target,DUEL_RESULT_VICTORY,
					&_duel_target,sizeof(_duel_target));
				
			time_t t = time(NULL);
			char buf[32];
			strftime(buf, sizeof(buf), "%F %T", localtime(&t)); 
			GLog::log(GLOG_INFO, "duel:time(%s),player1(%d),player2(%d),winner(%d)", buf, pImp->_parent->ID.id, _duel_target.id, _duel_target.id);
					
		}
		ClearDuelMode(pImp);
		_duel_target = XID(-1,-1);
	}
	else
	{
		//非决斗失败,也需要进行结束决斗的操作
		if(IsDuelStarted())
		{
			__PRINTF("Duel OnDeath \n");
			pImp->_runner->duel_result(_duel_target, false);

			//发出决斗结束的消息
			pImp->SendTo<0>(GM_MSG_PLAYER_DUEL_STOP,_duel_target,DUEL_RESULT_DEUCE,
					&_duel_target,sizeof(_duel_target));
				
			time_t t = time(NULL);
			char buf[32];
			strftime(buf, sizeof(buf), "%F %T", localtime(&t)); 
			GLog::log(GLOG_INFO, "duel:time(%s),player1(%d),player2(%d),winner(0)", buf, pImp->_parent->ID.id, _duel_target.id);

			ClearDuelMode(pImp);
			_duel_target = XID(-1,-1);
		}
	}
}

void 
player_duel::SetDuelPrepareMode()
{
	_duel_mode = 1;
}

void 
player_duel::SetDuelStartMode(gplayer_imp * pImp)
{
	_duel_mode = 2;
	_duel_timeout = DUEL_TIME_LIMIT;
	//加入一个filter 
	object_interface oif(pImp);
	oif.AddFilter(new pvp_duel_filter(pImp,_duel_target,FILTER_INDEX_DUEL_FILTER));

}

void 
player_duel::ClearDuelMode(gplayer_imp * pImp)
{
	_duel_mode = 0;
	if(pImp->_filters.IsFilterExist(FILTER_INDEX_DUEL_FILTER))
	{
		//删除不需要的filter
		//通知大家自己离开了决斗模式 在filter 中完成吧 

		//pImp->_filters.RemoveFilter(FILTER_INDEX_DUEL_FILTER);

		//再加入一个新的filter，表明需要有暂时5秒的无敌
		//ASSERT(_duel_target.IsValid());
		pImp->_filters.ModifyFilter(FILTER_INDEX_DUEL_FILTER,FMID_DUEL_END,NULL,0);
	}
}

bool 
player_duel::IsPrepareMode()
{
	return _duel_mode == 1;
}

bool 
player_duel::IsDuelStarted()
{
	return _duel_mode == 2;
}
	
