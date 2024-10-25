#ifndef __ONLINEGAME_GS_PLAYER_DUEL_H__
#define __ONLINEGAME_GS_PLAYER_DUEL_H__

#include <common/types.h>
#include <string.h>
class gplayer_imp;
class player_duel
{
protected:
	XID  _duel_target;	//和谁决斗
	int  _duel_timeout;	//决斗的超时时间
	int  _duel_mode;	//是否决斗状态 0 无, 1 等待 2 开始

	struct
	{
		XID 	duel_target;	//邀请目标
		int	timeout;	//邀请超时记录
		bool 	is_invite;	//是否邀请了对象
		bool 	is_agree_duel;	//是否同意了决斗
	} _invite;

	enum 
	{
		DUEL_INVITE_TIMEOUT 	= 30,
		DUEL_TIME_LIMIT 	= 600,

		DUEL_RESULT_TIMEOUT	= 0,
		DUEL_RESULT_VICTORY	= 1,
		DUEL_RESULT_DEUCE	= 2,

	};

	void SetDuelPrepareMode();
	void SetDuelStartMode(gplayer_imp * pImp);
	void ClearDuelMode(gplayer_imp * pImp);
	bool IsPrepareMode();
	bool IsDuelStarted();

public:	
	enum {
		DUEL_REPLY_SUCCESS	= 0,
		DUEL_REPLY_TIMEOUT	= 1,
		DUEL_REPLY_REJECT	= 2,
		DUEL_REPLY_OUT_OF_RANGE	= 3,
		DUEL_REPLY_ERR_STATUS	= 4,
	};


	player_duel():_duel_target(-1,-1),_duel_timeout(0),_duel_mode(false)
	{
		memset(&_invite,0,sizeof(_invite));
	}

	template <typename WRAPPER> void Save(WRAPPER & wrapper)
	{
		wrapper.push_back(this, sizeof(*this));
	}

	template <typename WRAPPER> void Load(WRAPPER & wrapper)
	{
		wrapper.pop_back(this, sizeof(*this));
	}

	void Swap(player_duel & rhs)
	{
		player_duel tmp = rhs;
		rhs = * this;
		*this = tmp;
	}
	
	inline bool IsDuelMode()
	{
		return _duel_mode;
	}

	inline const XID & GetDuelTarget()
	{
		return _duel_target;
	}

	void PlayerDuelRequest(gplayer_imp * pImp,const XID & target);
	void PlayerDuelReply(gplayer_imp * pImp,const XID & target,int param);

	void MsgDuelRequest(gplayer_imp * pImp,const XID & who);
	void MsgDuelReply(gplayer_imp * pImp,const XID & who,int param);
	void MsgDuelPrepare(gplayer_imp * pImp,const XID & who);
	void MsgDuelCancel(gplayer_imp * pImp, const XID & who);
	void MsgDuelStart(gplayer_imp * pImp, const XID & who);
	void MsgDuelStop(gplayer_imp * pImp, const XID & who,int param);

	void Heartbeat(gplayer_imp *pImp);
	void OnDeath(gplayer_imp * pImp, bool duel_failed);

/*
	PlayerDuelRequest ---> MSG ---> MsgDuelRequest
	PlayerDuelReply ---> MSG ---> MsgDuelReply
*/
	

};

#endif

