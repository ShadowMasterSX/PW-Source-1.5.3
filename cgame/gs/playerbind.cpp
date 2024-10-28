#include "playerbind.h"
#include "world.h"
#include "player_imp.h"
#include "obj_interface.h"
#include "duel_filter.h"
#include "filter_man.h"

void
player_bind::PlayerLinkRequest(gplayer_imp * pImp, const XID & target)
{
	if(!pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}
	
	//检测自己的状态
	//能够发起请求的条件是 1:不在马上 2:不在别人身上 3:没有在要求中 4:不在水中 5:其他...
	if(_mode || !pImp->CheckPlayerBindRequest() || !target.IsPlayer())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//设置状态
	_mode = MODE_REQUEST;
	_request_target = target;
	_timeout = NORMAL_TIMEOUT;

	//发出消息
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_REQUEST,target,0);

}

void
player_bind::PlayerLinkInvite(gplayer_imp * pImp, const XID & target)
{
	if(pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))//禁止相依相偎
		return;
	
	if(pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}

	//检测自己的状态
	//能够发起邀请的条件是 1:不在马上 2:不在别人身上 3:没有在要求中 4:不在水中 5:其他...
	if(_mode || !pImp->CheckPlayerBindInvite() || !target.IsPlayer() )
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//设置状态
	_mode = MODE_INVITE;
	_invite_target = target;
	_timeout = NORMAL_TIMEOUT;

	//发出消息
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_INVITE,target,0);
}

void
player_bind::PlayerLinkReqReply(gplayer_imp * pImp, const XID & target,int param)
{
	if(pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}

	//检测自己的状态
	//能够回应请求的条件和能够邀请的条件类似
	if((_mode && _mode != MODE_INVITE ) || !pImp->CheckPlayerBindInvite() || !target.IsPlayer() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//设置状态
	_mode = MODE_PREPARE;
	_bind_target = target;
	_timeout = PREPARE_TIMEOUT;

	//发出消息
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_REQUEST_REPLY,target,param);
}

void
player_bind::PlayerLinkInvReply(gplayer_imp * pImp, const XID & target,int param)
{
	if(!pImp->IsPlayerFemale())
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_GENDER);
		return;
	}

	//检测自己的状态
	//能够回应请求的条件和能够请求的条件类似
	if((_mode && _mode != MODE_REQUEST) || !pImp->CheckPlayerBindRequest() || !target.IsPlayer() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_BIND_REQUEST);
		return;
	}

	if(world_manager::GetWorldLimit().nobind)
	{
		pImp->_runner->error_message(S2C::ERR_CANNOT_BIND_HERE);
		return;
	}

	//设置状态
	_mode = MODE_PREPARE;
	_bind_target = target;
	_timeout = PREPARE_TIMEOUT;

	//发出消息
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_INVITE_REPLY,target,param);

}


void 
player_bind::MsgRequest(gplayer_imp * pImp, const XID & target)
{
	//男
	if(pImp->IsPlayerFemale()) return;

	//收到请求后的操作是检查是否处于可以被请求的状态，如果不能，直接返回一个拒绝即可
	//有关距离的判断都在外面完成
	//目前的检查是应当处于非被骑和等待的状态即可
	if(_mode && ( _mode == MODE_LINKED || _mode == MODE_PREPARE)
		|| ((gplayer*)pImp->_parent)->IsInvisible()
		|| pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//直接返回一个拒绝消息即可
		pImp->SendTo<0>(GM_MSG_PLAYER_BIND_REQUEST_REPLY,target,ERR_CANNOT_BIND);
		return;
		
	}

	//通知消息给客户端
	pImp->_runner->player_bind_request(target);
}

void 
player_bind::MsgInvite(gplayer_imp * pImp, const XID & target)
{
	//女
	if(!pImp->IsPlayerFemale()) return;


	//收到邀请后的操作和请求类似
	if(_mode && ( _mode == MODE_LINKED || _mode == MODE_PREPARE)
		|| ((gplayer*)pImp->_parent)->IsInvisible()
		|| pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//直接返回一个拒绝消息即可
		pImp->SendTo<0>(GM_MSG_PLAYER_BIND_INVITE_REPLY,target,ERR_CANNOT_BIND);
		return;
		
	}

	//通知消息给客户端
	pImp->_runner->player_bind_invite(target);

}

void 
player_bind::MsgRequestReply(gplayer_imp * pImp, const XID & target, int param)
{
	//女
	//收到请求的回应，能够处理的前提是自己的请求对象是当前对象
	//且当前处于请求状态
	if(target != _request_target || _mode != MODE_REQUEST || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//回馈一个消息 让对方脱离等待状态
		if(!param) pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		return;
	}
	
	//如果传回来的是拒绝消息，则清除当前状态，进行必要的操作
	if(param)
	{
		//此操作要传回给客户端
		pImp->_runner->player_bind_request_reply(target,param);

		//清除当前的邀请状态
		_mode = MODE_EMPTY;
		_request_target = XID(-1,-1);
		return;
	}
	
	_mode = MODE_PREPARE;
	_bind_target = target;
	_timeout = PREPARE_TIMEOUT;

	//传回的是成功消息,则通告之,并且通知对方进入可以进入绑定状态,然后让男方发起绑定
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_PREPARE,target,0);
}

void 
player_bind::MsgInviteReply(gplayer_imp * pImp, const XID & target, int param)
{
	//男
	//收到邀请的回应,处理的条件是邀请的人和返回对应
	//且当前处于请求状态
	if(target != _invite_target || _mode != MODE_INVITE || !pImp->CheckPlayerBindRequest() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//回馈一个消息 让对方脱离等待状态
		if(!param) pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		return;
	}
	
	//如果传回来的是拒绝消息，则清除当前状态，进行必要的操作
	if(param)
	{
		//此操作要传回给客户端
		pImp->_runner->player_bind_invite_reply(target,param);

		//清除当前的邀请状态
		_mode = MODE_EMPTY;
		_invite_target = XID(-1,-1);
		return;
	}
	if(!pImp->EnterBindMoveState(target))
	{
		//进入状态失败 则直接返回
		return ;
	}
	//由对方发送绑定开始的消息

	_mode = MODE_LINKED;
	_bind_target = target;
	//对方同意开始绑定了,直接进入绑定状态

	//通知对方开始绑定
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_LINK,target,0);
}

void 
player_bind::MsgPrepare(gplayer_imp *pImp, const XID & target)
{
	//能收到此消息的必然是男性, 同时必然处于等待玩家的状态 MODE_PREPARE
	if(_mode != MODE_PREPARE || target != _bind_target || !pImp->CheckPlayerBindRequest() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//状态不匹配或者目标不匹配则忽略
		return;
	}
	//检查一下是否为男性
	ASSERT(!pImp->IsPlayerFemale());

	//对方同意开始绑定了,直接进入绑定状态
	if(!pImp->EnterBindMoveState(target))
	{
		//进入状态失败 则直接返回
		return ;
	}
	//状态一致,进入link的状态
	_mode = MODE_LINKED;
	//由对方发送绑定开始的消息
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_LINK,target,0);
}

void 
player_bind::MsgBeLinked(gplayer_imp * pImp, const XID & target,const A3DVECTOR & pos)
{
	if(_mode != MODE_PREPARE || _bind_target != target || !pImp->CheckPlayerBindRequest() || pImp->GetPlayerLimit(PLAYER_LIMIT_NOBIND))
	{
		//若是非预期的状态则不处理
		//考虑一下自己的状态是否符合 (是否处于可以进行移动的状态,这个状态和请求的状态一致)
		//是否返回一个消息?
		pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		return;
	}

	if(!pImp->EnterBindFollowState(target))
	{
		//进入状态失败则直接返回
		return;
	}
	
	_mode = MODE_FOLLOW;
	A3DVECTOR tmp = pos;
	tmp -= pImp->_parent->pos;
	pImp->StepMove(tmp);

	pImp->_runner->player_bind_start(target);
	//进入绑定状态
	
}

void 
player_bind::MsgFollowOther(gplayer_imp * pImp, const XID & target,const A3DVECTOR & pos)
{
	//其他玩家发来了要求跟随的消息
	if(_mode != MODE_FOLLOW || target != _bind_target)
	{
		//不予处理
		//或者考虑返回一个停止link
		if(target != _bind_target || _mode != MODE_PREPARE)
		{
			//这是担心消息顺序错误,不是所有的状态都返回错误消息
			pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,target, 0);
		}
		return ;
	}
	//否则进行移动
	A3DVECTOR tmp = pos;
	tmp -= pImp->_parent->pos;
	pImp->StepMove(tmp);
//	pImp->_runner->stop_move(pos,0x7F00,0,0);
}

void 
player_bind::MsgStopLinked(gplayer_imp * pImp, const XID & target)
{
	//收到取消link的消息 
	if((_mode == MODE_LINKED || _mode == MODE_FOLLOW ) && target == _bind_target)
	{
		pImp->_runner->player_bind_stop();
		pImp->ReturnBindNormalState();
		_mode = MODE_EMPTY;
		_bind_target = XID(-1,-1);
	}
}


void 
player_bind::Heartbeat(gplayer_imp * pImp)
{
	if(!_mode) return ;
	switch(_mode)
	{
		case MODE_REQUEST:
		_timeout --;
		if(_timeout <=0)
		{
			//请求超时，可以考虑返回客户端一个消息
			//同时清空当前状态
			_mode = MODE_EMPTY;
			_request_target = XID(-1,-1);
		}
		break;
		
		case MODE_INVITE:
		_timeout --;
		if(_timeout <=0)
		{
			//请求超时，可以考虑返回客户端一个消息
			//同时清空当前状态
			_mode = MODE_EMPTY;
			_invite_target = XID(-1,-1);
		}
		break;

		case MODE_PREPARE:
		_timeout --;
		if(_timeout <=0)
		{
			//请求超时，可以考虑返回客户端一个消息
			//同时清空当前状态
			_mode = MODE_EMPTY;
			_bind_target = XID(-1,-1);
		}
		break;

		case MODE_LINKED:
		case MODE_FOLLOW:
		//这两种状态需要检测距离的远近 ，同时作出处理
		{
			//没有超时，进行范围和位置的判断
			world::object_info info;
			if(!pImp->_plane->QueryObject(_bind_target,info)
					|| (info.state & world::QUERY_OBJECT_STATE_ZOMBIE)
					|| (info.pos.squared_distance(pImp->_parent->pos)) >= 50.f*50.f)
			{
				//如果查询不到对象,或者距离过远 则主动退出结束绑定
				pImp->ReturnBindNormalState();
				pImp->_runner->player_bind_stop();
				_mode = MODE_EMPTY;
				_bind_target = XID(-1,-1);
			}
			else
			{
				//每秒长元气5点
				pImp->ModifyAP(5);
			}
		}

		break;
	}
}

void 
player_bind::PlayerCancel(gplayer_imp *pImp)
{
	ASSERT(_mode == MODE_LINKED || _mode == MODE_FOLLOW);

	//给对方发送一个停止绑定的消息
	pImp->SendTo<0>(GM_MSG_PLAYER_BIND_STOP,_bind_target, 0);
	
	//转移到正常状态 
	pImp->ReturnBindNormalState();
	pImp->_runner->player_bind_stop();
	_mode = MODE_EMPTY;
	_bind_target = XID(-1,-1);
}

