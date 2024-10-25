#ifndef __ONLINEGAME_GS_PLAYER_GIFT_CARD_H__
#define __ONLINEGAME_GS_PLAYER_GIFT_CARD_H__
#include "common/packetwrapper.h"
/**
 *	����ౣ���������Ʒ��һ�ʱ��Ҫ�����һЩ״̬�ʹ�������
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
		GCR_SUCCESS,			// �ɹ�
		GCR_UNACTIVE_CARD,		// δ����
		GCR_PRE_LIMIT,			// �������ظ�
		GCR_COMPLETE,			// �����
		GCR_NOT_OWNER,			// ��ӵ��
		GCR_INVALID_CARD,		// ��Ч����
		GCR_OUT_DATE,			// ����
		GCR_OTHER_USED,			// ������ɫ��ʹ��
		GCR_NET_FAIL,			// ���³���
		GCR_NOT_BIND,			// δ���˺�
		GCR_UNMARSHAL_FAIL,		// ���ݴ���
		GCR_INVALID_USER,		// ��Ч�˺�
		GCR_TYPE_LIMIT,			// �����ظ�
		GCR_INVALID_ROLE,		// ��Ч��ɫ

		GCR_WAIT_COOLDOWN,		// �ȴ���ȴ
		GCR_WAIT_TASK,			// �ȴ�����ɹ�
		GCR_WAIT_AU,			// ����һ������
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
	} data; // ������Ϣ 
	
	int		_timeout;
	int     _noticepass;
};

#endif
