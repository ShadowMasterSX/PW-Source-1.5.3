#ifndef __GNET_TRADE_H
#define __GNET_TRADE_H
#include <vector>
#include <set>
#include <map>

#include "octets.h"
#include "thread.h"
#include "itimer.h"
#include "macros.h"
#include "errcode.h"
#include "groleinventory"
#include "grolepocket"

#define	TRADER_SERVER 0

//物品属性的定义：(true的含义)
//0. 死亡时不能掉落
//1. 不可以仍在地上
//2. 不可以卖给NPC
//3. 记录详细log信息
//4. 不可以玩家间交易
//5. 是任务相关物品

#define __GOOD_NODROP_WHENDEAD	(1<<0)
#define __GOOD_NODROP_ONFLOOR	(1<<1)
#define __GOOD_NOSELL_TONPC	(1<<2)
#define __GOOD_LOG_DETAIL	(1<<3)
#define __GOOD_NOTRADE_PLAYER	(1<<4)
#define __GOOD_ISTASKGOOD	(1<<5)

namespace GNET
{
inline bool operator==(const GRoleInventory &inventory1, const GRoleInventory &inventory2)
{
	return inventory1.id == inventory2.id && inventory1.pos == inventory2.pos;	
}

typedef std::vector<GRoleInventory> TradeItems;

class Transaction;
class Trader
{
	friend class Transaction;
public:
	int roleid;
	unsigned int linksid;
	unsigned int localsid;

	enum Options
	{ 
		OPT_ATTACHTRADER,
		OPT_SETPSSN, 
		OPT_UPDATEGOODS, 
		OPT_SUBMIT, 
		OPT_CONFIRM, 
		OPT_DISCARD, 
		OPT_MOVEOBJ 
	};

private:
	enum State
	{ 
		INIT, 
		INIT_HALF, 
		EXCHANGE, 
		READY, 
		CONFIRM 
	};

	unsigned int 	tid;			//Transaction ID, which this trader joined in	
	bool		pssn_modified;		//Trader's possession is modified
	Trader::State	state;
	
	unsigned int	capacity;		//pocket capacity
	unsigned int 	exchg_money;  		//exchange money	
	unsigned int	pssn_money;		//possess money
	TradeItems	exchg_obj;		//exchange objects
	TradeItems	possession;		//possess objects
	
	explicit Trader(int _roleid, unsigned int _linksid, unsigned int _localsid) : roleid(_roleid), linksid(_linksid), 
		      localsid(_localsid),tid(0),pssn_modified(false),state(INIT),exchg_money(0),pssn_money(0) 
	{  }	

	void SetPossession(GRolePocket& pocket);

	Trader::State GetState() { return state; }
	void ChangeState(Trader::State _state) { state=_state; }
	bool StatePolicy(Trader::Options opt) const
	{ 
		if (state==READY && opt==OPT_UPDATEGOODS) return false;
		if (state==CONFIRM && opt==OPT_DISCARD) return false;
		if (opt==OPT_CONFIRM && state!=READY) return false;
		if (opt==OPT_MOVEOBJ && state!=EXCHANGE) return false;
		return true;
	}
	
	bool VerifyGoods(const GRoleInventory& goods,const TradeItems& goods_list);	
	bool AddExchgObject(GRoleInventory& goods,unsigned int money);
	bool RemoveExchgObject(const GRoleInventory& goods,unsigned int money);
	bool MoveObject(const GRoleInventory& goods,int toPos);

	bool Submit(); 
	bool Confirm();
	void Show();
	
	bool MergeInventory(const TradeItems & toFill, TradeItems & result);
public:
	~Trader() 
	{ 
		exchg_obj.clear();
		possession.clear();
	}
};

class Transaction : public IntervalTimer::Observer
{
public:
	class State
	{
		int		timeout;
		std::set<Trader::Options> set;
	public:
		State(int t) : timeout(t) { }
		State(int t,Trader::Options* first,size_t size) : timeout(t),set(first,first+size) { }
		bool TimePolicy(int elapse_time) { return timeout==-1 ? true:elapse_time<timeout; }
		bool OptionPolicy(Trader::Options opt) { return set.find(opt) != set.end(); }
	};	

	typedef void (*CallBack) (unsigned int,bool/*timeout*/,bool/*Alice modified*/,bool/*Bob modified*/);
   	CallBack OnDestroy;
	int gs_id;	//game server id
private:
	unsigned int tid;
	IntervalTimer 	m_timer;
	
	Trader* Alice;
	Trader* Bob;
	
	Transaction::State* state;

	bool			blGenResult;	//whether generate transaction result
	unsigned int		_newmoney_alice;//new money value of alice after transaction
	unsigned int 		_newmoney_bob;	//new money value of bob after transaction
	TradeItems		_newposs_alice;	//possess of alice after transaction
	TradeItems 		_newposs_bob;	//possess of bob after transaction

	typedef	std::map<unsigned int/*tid*/,Transaction* > TransactionMap;
	static TransactionMap& GetTransMap() { static TransactionMap trans_map; return trans_map; }

	typedef	std::map<int,Trader*> TraderMap;
	static TraderMap& GetTraderMap() { static TraderMap trader_map; return trader_map; }
	static Thread::Mutex locker_map;
	
	Thread::Mutex locker_self;
	
	Transaction() : Alice(NULL),Bob(NULL),state(NULL),locker_self("Transaction::locker_self") { 
		blGenResult=false; 
		OnDestroy=NULL;
	}
	
	void ChangeState(Transaction::State* _state) { state=_state; m_timer.Reset(); }
	Trader* FindTrader(int roleid) 
	{
		if (Alice != NULL && Alice->roleid==roleid) return Alice;
		if (Bob != NULL && Bob->roleid==roleid) return Bob;
		return NULL;
	}
	void ReleaseObject()
	{
		{
			Thread::Mutex::Scoped l(locker_map);
			TraderMap::iterator it;
			if (Alice != NULL) 
			{
				it=GetTraderMap().find(Alice->roleid);
				if ( it!=GetTraderMap().end() && (*it).second->tid==tid) 
					GetTraderMap().erase(it);
			}
			if (Bob != NULL) 
			{
				it=GetTraderMap().find(Bob->roleid);
				if ( it!=GetTraderMap().end() && (*it).second->tid==tid) 
					GetTraderMap().erase(it);
			}
			GetTransMap().erase(tid);
		}
		if (Alice != NULL) delete Alice;
		if (Bob != NULL) delete Bob;
		delete this;
	}
public:
	bool GenerateResult(TradeItems &possesion, unsigned int* money, Trader* fromTrader,Trader* toTrader);
	static Transaction* Create(int roleid1, unsigned int linksid1, unsigned int localsid1, 
		int roleid2, unsigned int linksid, unsigned int localsid2);
	static Transaction* GetTransaction(unsigned int tid)
	{
		Thread::Mutex::Scoped l(locker_map);
		TransactionMap::iterator it=GetTransMap().find(tid);
		if (it==GetTransMap().end()) 
			return NULL;
		else
			return (*it).second;
	}
	static bool VerityTraders(int roleid1, int roleid2)
	{
		Thread::Mutex::Scoped l(locker_map);
		return GetTraderMap().find(roleid1)==GetTraderMap().end() 
			&& GetTraderMap().find(roleid2)==GetTraderMap().end() && roleid1!=roleid2;
	}	
	static Transaction* FindTransactionbyTrader(int roleid)
	{
		Thread::Mutex::Scoped l(locker_map);
		TraderMap::const_iterator it=GetTraderMap().find(roleid);
		if (it == GetTraderMap().end()) return NULL;
		
		TransactionMap::iterator it2=GetTransMap().find(it->second->tid);
		if (it2==GetTransMap().end()) 
			return NULL;
		else
			return (*it2).second;

	}
	bool Update();
	
	unsigned int GetTid() { return tid; }
	int AttachTrader(int roleid);

	Trader * GetAlice() { return Alice;}
	Trader * GetBob()   { return Bob;}

	int SetPossession(int roleid1,GRolePocket &p1,int roleid2,GRolePocket &p2);

	int AddExchgObject(int roleid, GRoleInventory &goods, unsigned int money);
	int RemoveExchgObject(int roleid, const GRoleInventory &goods, unsigned int money);
	int MoveObject(int roleid, const GRoleInventory &goods, int toPos);
	int Submit(int roleid, int* problem_roleid=NULL);
	int Confirm(int roleid, int* problem_roleid=NULL);
	int Discard();
	
	bool GetExchgResult(TradeItems &exchg1,unsigned int* money1,TradeItems &exchg2,unsigned int *money2,bool &blSave);
	void ShowTrader();
	void LogTrade();
	
	~Transaction() { }
	void Destroy(bool blRollback=false);
	static void DiscardTransaction(int roleid);
};

}; //end of namespace
#endif
