#ifndef __ONLINE_GAME_GS_PLAYER_BOUND_H__
#define __ONLINE_GAME_GS_PLAYER_BOUND_H__

#include <common/types.h>

//将两个玩家绑定在一起的过程控制
class gplayer_imp;
class player_bind
{
	XID _invite_target;

	XID _request_target;

	int _mode;
	XID _bind_target;
	int _timeout;

	enum 
	{
		MODE_EMPTY,
		MODE_REQUEST,
		MODE_INVITE,
		MODE_PREPARE,
		MODE_LINKED,
		MODE_FOLLOW,
	};

	enum 
	{
		NORMAL_TIMEOUT = 30,
		PREPARE_TIMEOUT = 5,
	};

	enum 
	{
		ERR_SUCCESS,
		ERR_REFUSE,
		ERR_CANNOT_BIND,
	};

public:
	player_bind():_invite_target(-1,-1), _request_target(-1,-1),
		      _mode(0),_bind_target(-1,-1),_timeout(0)
	{}

	template <typename WRAPPER> void Save(WRAPPER & wrapper)
	{
		wrapper.push_back(this, sizeof(*this));
	}

	template <typename WRAPPER> void Load(WRAPPER & wrapper)
	{
		wrapper.pop_back(this, sizeof(*this));
	}
	
	void Swap(player_bind & rhs)
	{
		player_bind tmp = rhs;
		rhs = * this;
		*this = tmp;
	}

	void PlayerLinkRequest(gplayer_imp * pImp, const XID & target);
	void PlayerLinkInvite(gplayer_imp * pImp, const XID & target);
	void PlayerLinkReqReply(gplayer_imp * pImp, const XID & target,int param);
	void PlayerLinkInvReply(gplayer_imp * pImp, const XID & target,int param);
	void PlayerCancel(gplayer_imp *pImp);


	void MsgRequest(gplayer_imp * pImp, const XID & target);
	void MsgInvite(gplayer_imp * pImp, const XID & target);
	void MsgRequestReply(gplayer_imp * pImp, const XID & target, int param);
	void MsgInviteReply(gplayer_imp * pImp, const XID & target, int param);
	void MsgPrepare(gplayer_imp *pImp, const XID & target);
	void MsgBeLinked(gplayer_imp * pImp, const XID & target,const A3DVECTOR & pos);
	void MsgFollowOther(gplayer_imp * pImp, const XID & target,const A3DVECTOR & pos);
	void MsgStopLinked(gplayer_imp * pImp, const XID & target);

	bool IsPlayerLinked() { return _mode == MODE_LINKED;}
	const XID & GetLinkedPlayer() { return _bind_target;}

	void Heartbeat(gplayer_imp * pImp);
	void OnUnderWater(gplayer_imp* pImp, float offset)
	{
		if(_mode != MODE_LINKED) return;
		if(offset > 1.0f) PlayerCancel(pImp);
	}
};

#endif

