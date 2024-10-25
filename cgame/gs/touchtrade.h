#ifndef __ONLINEGAME_GS_TOUCH_TRADE_H__
#define __ONLINEGAME_GS_TOUCH_TRADE_H__
#include "common/packetwrapper.h"
/**
 *	这个类保存在玩家Touch交易时需要保存的一些状态和处理内容
 */

#define TOUCH_CAST_WAIT_TIME 60*2
#define TOUCH_POCKET_WAIT_TIME 30
#define NOTICE_INTERVAL 10

struct touch_trade 
{
public:
	enum
	{
		TPC_SUCCESS,
        TPC_ORDER_CLASH,
        TPC_NEED_MORE,
        TPC_COMPLETE,
        TPC_NEED_GOLD,
        TPC_BUSY,
        TPC_NO_ACCOUNT,
        TPC_OTHER,
       	TPC_NEED_SLOT = 128,
		TPC_FAIL = 255
	};

	enum
	{
		TSTATE_FREE,
		TSTATE_HALFTRADE,
		TSTATE_WAITPOCKET,
	};
public:
	struct db_save_data
	{
		int64_t 	sn;
		char        state;
		unsigned int cost;
		unsigned int lots;
		int itemtype;
		unsigned int itemcount;
		int itemexpire;
	} data; // 存盘信息 对应rpc:GTouchTrade结构

	int64_t		income; //总获得点数 au 存盘
	int64_t		remain; //可消费点数 au 存盘 

	touch_trade() : income(0),remain(0),_itemindex(0),
					_noticepass(0),_timeout(0),_roleid(0)
					{ memset(&data,0,sizeof(data)); }
	
	bool Save(archive & ar)
	{
		ar << data.sn;
		ar << data.state;
		ar << data.cost;
		ar << data.lots;
		ar << data.itemtype ;
		ar << data.itemcount;
		ar << data.itemexpire;
		ar << income;
		ar << remain;
		ar << _itemindex;
		ar << _noticepass;
		ar << _timeout;
		ar << _roleid;
		return true;
	}
	bool Load(archive & ar)
	{
		ar >> data.sn;
		ar >> data.state;
		ar >> data.cost;
		ar >> data.lots;
		ar >> data.itemtype;
		ar >> data.itemcount;
		ar >> data.itemexpire;
		ar >> income;
		ar >> remain;
		ar >> _itemindex;
		ar >> _noticepass;
		ar >> _timeout;
		ar >> _roleid;
		return true;
	}
	void Swap(touch_trade & rhs)
	{
		abase::swap(data, rhs.data);
		abase::swap(income, rhs.income);
		abase::swap(remain, rhs.remain);
		abase::swap(_itemindex, rhs._itemindex);
		abase::swap(_noticepass, rhs._noticepass);
		abase::swap(_timeout, rhs._timeout);
		abase::swap(_roleid, rhs._roleid);
	}

public:
	void 	OnHeartbeat(gplayer_imp * pImp);
	bool	OnIdClash(gplayer_imp * pImp);
	bool	Complete(gplayer_imp * pImp);
	bool 	Query(gplayer_imp * pImp);
	bool 	TryCost(gplayer_imp * pImp,unsigned int index ,int type,unsigned int count,unsigned int price,int expire,unsigned int lots);
	void 	SetTimeOut(int t)	{ _timeout = t;	}
	bool	IsFree() { return data.state == TSTATE_FREE; }
	bool	IsHalfTrade(int64_t sn ) { return data.sn == sn && data.state == TSTATE_HALFTRADE; }
	void 	ClearData();

public:
	void 	InitData(int64_t sn,char state,unsigned int cost,int type,
			         unsigned int count,unsigned int lots,int expire,unsigned int roleid)
	{
		data.sn	= sn;
		data.state = state;
		data.cost = cost;
		data.itemtype = type;
		data.itemcount = count;
		data.itemexpire = expire;
		data.lots = lots;
		
		income = 0;
		remain = 0;

		_timeout = 0;
		_noticepass = 0;

		_roleid = roleid;
		
		if(sn && (int)sn != _roleid) // error roleid in old sn
		{
			//	log	
			GLog::log(LOG_ERR,"roleid:%d sn %lld load warning",_roleid,sn);
		}
	}

	void 	GetData(int64_t &sn,char &state,unsigned int& cost,
			int &type,unsigned int &count,int& expire,unsigned int &lots )
	{
		sn    =	data.sn;
		state = data.state;
		cost  =	data.cost;
		type  =	data.itemtype;
		count = data.itemcount;
		expire= data.itemexpire;
		lots  = data.lots;
	}

protected:
	static int64_t	MakeNewSN(int roleid);
protected:	
	void 	OnHalfCost(gplayer_imp * pImp);
	void 	OnWaitPocket(gplayer_imp * pImp);
	void 	PurchaseItem(gplayer_imp * pImp,int type,int count,int expire,int lots);
private:
	unsigned int _itemindex;
	int		_noticepass;
	int 	_timeout;
	int 	_roleid;
};

#endif
