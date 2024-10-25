#ifndef __ONLINEGAME_GS_PLAYER_GIFT_CARD_H__
#define __ONLINEGAME_GS_PLAYER_GIFT_CARD_H__
#include "common/packetwrapper.h"
/**
 *	这个类保存在玩家礼品码兑换时需要保存的一些状态和处理内容
 */


class player_giftcard 
{
public:
	static const unsigned int GIFT_CARDNUMBER_LEN  = 20;
	static const unsigned int GIFT_WAITTASK_TIME  = 30;
	static const unsigned int GIFT_WAITREDEEM_TIME  = 120;
	static const unsigned int GIFT_NOTICE_INTERVAL  = 10;
	
	enum
	{
		GCR_SUCCESS,			// 成功
		GCR_UNACTIVE_CARD,		// 未激活
		GCR_PRE_LIMIT,			// 父类型重复
		GCR_COMPLETE,			// 已完成
		GCR_NOT_OWNER,			// 非拥有
		GCR_INVALID_CARD,		// 无效卡号
		GCR_OUT_DATE,			// 过期
		GCR_OTHER_USED,			// 其他角色已使用
		GCR_NET_FAIL,			// 重新尝试
		GCR_NOT_BIND,			// 未绑定账号
		GCR_UNMARSHAL_FAIL,		// 数据错误
		GCR_INVALID_USER,		// 无效账号
		GCR_TYPE_LIMIT,			// 类型重复
		GCR_INVALID_ROLE,		// 无效角色

		GCR_WAIT_COOLDOWN,		// 等待冷却
		GCR_WAIT_TASK,			// 等待任务成功
		GCR_WAIT_AU,			// 已有一个请求
	};
public:

	player_giftcard() : _timeout(0),_noticepass(0) 	{ memset(&data,0,sizeof(data)); }
	
	bool Save(archive & ar)
	{
		ar << data.state;
		ar << data.type;
		ar << data.parenttype;
		ar.push_back(data.cardnumber, sizeof(data.cardnumber));
		ar << _timeout;
		ar << _noticepass;

		return true;
	}
	bool Load(archive & ar)
	{
		ar >> data.state;
		ar >> data.type;
		ar >> data.parenttype;
		ar.pop_back(data.cardnumber,sizeof(data.cardnumber));
		ar >> _timeout;
		ar >> _noticepass;

		return true;
	}
	void Swap(player_giftcard & rhs)
	{
		abase::swap(data, rhs.data);
		abase::swap(_timeout, rhs._timeout);
		abase::swap(_noticepass, rhs._noticepass);
	}

public:
	void 	OnHeartbeat(gplayer_imp * pImp);
	void	OnRedeem(gplayer_imp * pImp, const char (&cardnumber)[GIFT_CARDNUMBER_LEN],int type,int parenttype,int retcode);
	
	bool	Complete(gplayer_imp * pImp);
	int		TryRedeem(gplayer_imp * pImp, const char (&cardnumber)[GIFT_CARDNUMBER_LEN]);
	void 	SetTimeOut(int t)	{ _timeout = t;	}
	bool	IsFree() { return data.state == GCR_STATE_FREE; }
	bool	IsHalfRedeem() { return data.state == GCR_STATE_HALFREDEEM; }
	void 	ClearData();

public:
	void 	InitData(char state,int type,int parenttype,const char (&cardnumber)[GIFT_CARDNUMBER_LEN])
	{
		data.state = state;
		data.type = type;
		data.parenttype = parenttype;
		strncpy(data.cardnumber,cardnumber,GIFT_CARDNUMBER_LEN);
	}

	void 	GetData(char& state,int& type,int& parenttype,char (&cardnumber)[GIFT_CARDNUMBER_LEN])
	{
		state = data.state;
		type  =	data.type;
		parenttype = data.parenttype;
		strncpy(cardnumber,data.cardnumber,GIFT_CARDNUMBER_LEN);
	}
protected:	
	enum
	{
		GCR_STATE_FREE,
		GCR_STATE_HALFREDEEM,
		GCR_STATE_WAITTASK,
	};

	void 	OnHalfRedeem(gplayer_imp * pImp);
	void 	OnWaitTask(gplayer_imp * pImp);
private:
	struct db_save_data
	{
		char        state;
		int			type;
		int			parenttype;
		char 		cardnumber[GIFT_CARDNUMBER_LEN];
	} data; // 存盘信息 
	
	int		_timeout;
	int     _noticepass;
};

#endif
