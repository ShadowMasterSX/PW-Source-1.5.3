#include "trade.h"
#include "../common/log.h"
#include <algorithm>
#include "gdeliveryserver.hpp"
#include "tradediscard_re.hpp"
namespace GNET
{
#define STATE_SIZE(x) sizeof(x)/sizeof(Trader::Options)

static Trader::Options _Init[] = 
{
	Trader::OPT_ATTACHTRADER
};
Transaction::State _TState_Init(300, _Init, STATE_SIZE(_Init));

static Trader::Options _Init_half[] = 
{
	Trader::OPT_ATTACHTRADER
};
Transaction::State _TState_Init_half(120, _Init_half, STATE_SIZE(_Init_half));

static Trader::Options _SetPossession[] = 
{
	Trader::OPT_SETPSSN
};
Transaction::State _TState_SetPossession(60, _SetPossession, STATE_SIZE(_SetPossession));

static Trader::Options _Exchange[] = 
{
	Trader::OPT_UPDATEGOODS,
	Trader::OPT_SUBMIT,Trader::OPT_DISCARD,
	Trader::OPT_MOVEOBJ
};
Transaction::State _TState_Exchange(600, _Exchange, STATE_SIZE(_Exchange));

static Trader::Options _Ready_half[] = 
{
	Trader::OPT_UPDATEGOODS,
	Trader::OPT_SUBMIT,
	Trader::OPT_DISCARD,
	Trader::OPT_MOVEOBJ
};
Transaction::State _TState_Ready_half(120, _Ready_half, STATE_SIZE(_Ready_half));

static Trader::Options _Ready[] = 
{
	Trader::OPT_CONFIRM,
	Trader::OPT_DISCARD
};
Transaction::State _TState_Ready(60, _Ready, STATE_SIZE(_Ready));

static Trader::Options _Confirm_half[] = 
{
	Trader::OPT_CONFIRM,
	Trader::OPT_DISCARD
};
Transaction::State _TState_Confirm_half(60, _Confirm_half, STATE_SIZE(_Confirm_half));

Transaction::State _TState_Confirm(-1);
Transaction::State _TState_Null(0);
Thread::Mutex Transaction::locker_map("Transaction::locker_map");

void Trader::SetPossession(GRolePocket& pocket)
{
	possession.swap(pocket.items.GetVector());
	pssn_money = pocket.money;
	capacity = pocket.capacity;
}

bool Trader::VerifyGoods(const GRoleInventory& goods,const TradeItems& goods_list)
{
	TradeItems::const_iterator it = std::find(goods_list.begin(), goods_list.end(), goods);
	if (it == goods_list.end()) return false;
	return 	(*it).count >= goods.count && goods.count>=0 && ((*it).proctype & __GOOD_NOTRADE_PLAYER) == 0;
}

bool Trader::AddExchgObject(GRoleInventory& goods,unsigned int money)
{
	if (! StatePolicy(OPT_UPDATEGOODS)) 
		return false;
	if( exchg_obj.size()>=24 || !capacity)
		return false;
	if (goods.id && goods.count>0)
	{
		if (! VerifyGoods(goods,possession) ) return false;
		//add to exchg_obj
		TradeItems::iterator it = std::find(exchg_obj.begin(), exchg_obj.end(), goods);
		TradeItems::iterator it2 = std::find(possession.begin(), possession.end(), goods);
		if (it == exchg_obj.end()) 
		{
			int tmp = goods.count;
			goods = *it2;
			goods.count = tmp;
			exchg_obj.push_back(goods);
		}
		else
			it->count += goods.count;
		//remove from possession
		it2->count -= goods.count;
	}
	if (money != 0) 
	{
		if (pssn_money < money) return false;
		exchg_money += money;
		pssn_money -= money;	
	}
	ChangeState(Trader::EXCHANGE);
	return true;
}
bool Trader::RemoveExchgObject(const GRoleInventory& goods,unsigned int money)
{
	if (! StatePolicy(OPT_UPDATEGOODS)) return false;
	if (goods.id &&  goods.count>0)
	{
		if (! VerifyGoods(goods,exchg_obj) ) return false;
		//remove from exchg_obj
		TradeItems::iterator it = std::find(exchg_obj.begin(),exchg_obj.end(),goods);
		if ((it->count -= goods.count) == 0) exchg_obj.erase(it);

		//add to possession
		it = std::find(possession.begin(), possession.end(), goods);
		if (it == possession.end()) return false;
		it->count += goods.count;
	}
	if (money != 0)
	{
		if (exchg_money < money ) return false;
		exchg_money -= money;
		pssn_money += money;
	}
	ChangeState(Trader::EXCHANGE);
	return true;
}
bool Trader::MoveObject(const GRoleInventory& goods,int toPos)
{
	if ( goods.pos==toPos || goods.count<=0 || toPos>=(int)capacity || toPos<0) 
		return false;
	if (! StatePolicy(OPT_MOVEOBJ) || !VerifyGoods(goods,possession))
		return false;

	//whether goods is in exchange container
	for (size_t i=0;i<exchg_obj.size();i++)
		if (exchg_obj[i].pos == goods.pos || exchg_obj[i].pos == toPos) return false;

	//find toPos Goods
	GRoleInventory* origin_goods=NULL;
	for (size_t i=0;i<possession.size();i++)
		if (possession[i].pos==toPos) {origin_goods=&possession[i]; break; }

	if (origin_goods==NULL)
	{
		//move directly
		TradeItems::iterator it = std::find(possession.begin(), possession.end(), goods);
		if (goods.count == (*it).count)  //move totallly
			(*it).pos=toPos;
		else
		{
			(*it).count -= goods.count;
			GRoleInventory new_goods((*it));
			new_goods.pos = toPos;
			new_goods.count = goods.count;
			possession.push_back(new_goods);
		}
	}
	else
	{
		//overlap check and compute
		if (origin_goods->id != goods.id) 
		{//exchange object
			TradeItems::iterator it=std::find(possession.begin(),possession.end(),goods);
			(*it).pos = toPos;
			origin_goods->pos = goods.pos;
			pssn_modified = true;
			return true;
		}
		if (origin_goods->count == origin_goods->max_count) return false;
		if (origin_goods->count + goods.count > origin_goods->max_count)
		{
			TradeItems::iterator it=std::find(possession.begin(),possession.end(),goods);
			int count = origin_goods->max_count - origin_goods->count;
			(*it).count -= count;
			origin_goods->count = origin_goods->max_count;
		}
		else
		{
			origin_goods->count += goods.count;
			TradeItems::iterator it=std::find(possession.begin(),possession.end(),goods);
			(*it).count -= goods.count;
			if ((*it).count == 0)
				possession.erase(it);
		}
	}
	pssn_modified = true;
	return true;
}
bool Trader::Submit()
{
	if (state == Trader::READY)
		return true;
	if (! StatePolicy(OPT_SUBMIT)) 
		return false;
	ChangeState(Trader::READY);
	return true;
}
bool Trader::Confirm()
{
	if (state == Trader::CONFIRM)
		return true;
	if (! StatePolicy(OPT_CONFIRM)) 
		return false;
	ChangeState(Trader::CONFIRM);
	return true;
}

bool Trader::MergeInventory(const TradeItems & toFill, TradeItems & result)
{
	result.clear();

	std::set<size_t> freeslot;
	for (size_t i=0; i < capacity; ++i)
		freeslot.insert(i);

	TradeItems::iterator iv = possession.begin();
	for (; iv != possession.end(); ++iv)
	{
		if (iv->count)
		{
			freeslot.erase(iv->pos);
			result.push_back(*iv);
		}
	}

	TradeItems::const_iterator it = toFill.begin();
	for (; it != toFill.end(); ++it)
	{
		int count = it->count;
		iv = result.begin();
		for (; iv != result.end(); ++iv)
		{
			if (iv->id == it->id && iv->count < iv->max_count)
			{
				int total = iv->count + count;
				if (total > iv->max_count)
				{
					count = total - iv->max_count;
					iv->count = iv->max_count;
				}
				else
				{
					iv->count += count;
					count = 0;
					break;
				}
			}
		}
		if (count)
		{
			if (!freeslot.size())
				return false;
			result.push_back(*it);
			GRoleInventory& item = result.back();
			item.count = count;
			item.pos = *(freeslot.begin());
			freeslot.erase(freeslot.begin());
		}
	}
	return true;
}

struct PrintInventory
{
	PrintInventory() {}
	void operator()(const GRoleInventory & item)
	{
		printf("\tid=%3d  pos=%3d  count=%3d  maxcount=%3d  property=%.*s\n", item.id, 
		item.pos, item.count, item.max_count, item.data.size(), (char*)item.data.begin()); 
	}
};

void Trader::Show()
{
	printf("\ntrader(id: %d)\n", roleid);

	printf("\tExchange Obj:\n");
	std::for_each(exchg_obj.begin(), exchg_obj.end(), PrintInventory());
	printf("\tmoney=%d\n", exchg_money);

	printf("\n\tPosession Obj:\n");
	std::for_each(possession.begin(), possession.end(), PrintInventory());
	printf("\tmoney=%d\n", pssn_money);
}

/************ memeber functions of Transaction **************/
bool Transaction::Update()
{ 
	if (!state->TimePolicy(m_timer.Elapse()))
	{
		if (OnDestroy != NULL && state != &_TState_Init)
			(*OnDestroy)(tid, state!=&_TState_Null, Alice->pssn_modified, Bob->pssn_modified);
		this->ReleaseObject();
		return false;
	}
	else
		return true;		
}

Transaction* Transaction::Create(int roleid1, unsigned int linksid1, unsigned int localsid1, 
		int roleid2, unsigned int linksid2, unsigned int localsid2)
{
	static unsigned int _tid=1;
	Transaction* t=new Transaction();
	t->tid=_tid++;
	Thread::Mutex::Scoped l(locker_map);
	GetTransMap()[t->tid]=t;
	t->ChangeState(&_TState_Init);
	t->Alice = new Trader(roleid1, linksid1, localsid1);
	t->Bob = new Trader(roleid2, linksid2, localsid2);
	//set tid to Trader
	t->Alice->tid = t->tid;
	t->Bob->tid = t->tid;

	IntervalTimer::Attach(t, 1000000/IntervalTimer::Resolution());
	return t;
}

int Transaction::AttachTrader(int roleid)
{
	Thread::Mutex::Scoped l(locker_self);
	if (!state->OptionPolicy(Trader::OPT_ATTACHTRADER)) return ERR_TRADE_INVALID_TRADER;

	Trader* trader = FindTrader(roleid);
	if (trader==NULL)
		return ERR_TRADE_INVALID_TRADER;

	{
		Thread::Mutex::Scoped l(locker_map);
		Transaction::TraderMap::iterator it=GetTraderMap().find(roleid);
		if (it != GetTraderMap().end()) 
			return ERR_TRADE_BUSY_TRADER;

		trader->ChangeState(Trader::INIT_HALF);
		GetTraderMap()[roleid]=trader;
	}	

	if (Alice->state != Trader::INIT_HALF || Bob->state != Trader::INIT_HALF )
	{
		ChangeState(&_TState_Init_half);
		return ERR_TRADE_ATTACH_HALF;
	}
	else
	{
		ChangeState(&_TState_SetPossession);
		return ERR_TRADE_ATTACH_DONE;
	}
}

int Transaction::SetPossession(int roleid1, GRolePocket &p1, int roleid2, GRolePocket &p2)
{
	Thread::Mutex::Scoped l(locker_self);
	if (!state->OptionPolicy(Trader::OPT_SETPSSN)) return ERR_TRADE_SETPSSN;

	Trader* t=FindTrader(roleid1);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER;
	t->SetPossession(p1);

	t=FindTrader(roleid2);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER;
	t->SetPossession(p2);

	//change transaction's state
	ChangeState(&_TState_Exchange);

	//change trader's state
	Alice->ChangeState(Trader::EXCHANGE);
	Bob->ChangeState(Trader::EXCHANGE);
	return ERR_SUCCESS;
}

bool CheckExchgParam(const GRoleInventory & goods, unsigned int money)
{
	return (goods.count<0 || (goods.id&&!goods.count) || (!goods.id&&!money))? false : true;
}

int Transaction::AddExchgObject(int roleid,GRoleInventory& goods,unsigned int money)
{
	Thread::Mutex::Scoped l(locker_self);
	Trader* t=FindTrader(roleid);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER;

	if (!state->OptionPolicy(Trader::OPT_UPDATEGOODS)) return ERR_TRADE_ADDGOODS;

	if (!CheckExchgParam(goods, money)) return ERR_TRADE_ADDGOODS;

	if (t->AddExchgObject(goods,money))
	{
		ChangeState(&_TState_Exchange);
		if (Alice->GetState() == Trader::READY) Alice->ChangeState(Trader::EXCHANGE);
		if (Bob->GetState() == Trader::READY) Bob->ChangeState(Trader::EXCHANGE);
		return ERR_SUCCESS;
	}
	else
		return ERR_TRADE_ADDGOODS;
}
int Transaction::RemoveExchgObject(int roleid, const GRoleInventory& goods,unsigned int money)
{
	Thread::Mutex::Scoped l(locker_self);
	Trader* t=FindTrader(roleid);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER;

	if (!state->OptionPolicy(Trader::OPT_UPDATEGOODS)) return ERR_TRADE_RMVGOODS;

	if (!CheckExchgParam(goods, money)) return ERR_TRADE_RMVGOODS;

	if (t->RemoveExchgObject(goods,money))
	{
		ChangeState(&_TState_Exchange);
		if (Alice->GetState() == Trader::READY) Alice->ChangeState(Trader::EXCHANGE);
		if (Bob->GetState() == Trader::READY) Bob->ChangeState(Trader::EXCHANGE);
		return ERR_SUCCESS;
	}
	else
		return ERR_TRADE_RMVGOODS;
}
int Transaction::MoveObject(int roleid, const GRoleInventory& goods, int toPos)
{
	Thread::Mutex::Scoped l(locker_self);
	Trader* t=FindTrader(roleid);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER; 
	if (!state->OptionPolicy(Trader::OPT_MOVEOBJ))
		return ERR_TRADE_MOVE_FAIL;
	if (!t->MoveObject(goods,toPos))
		return ERR_TRADE_MOVE_FAIL;
	return ERR_SUCCESS;
}

int Transaction::Submit(int roleid, int* problem_roleid)
{
	Thread::Mutex::Scoped l(locker_self);
	Trader* t=FindTrader(roleid);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER; 
	if (!state->OptionPolicy(Trader::OPT_SUBMIT))
		return ERR_TRADE_SUBMIT_FAIL;
	if (!t->Submit()) return ERR_TRADE_SUBMIT_FAIL;
	if (Alice->GetState() == Bob->GetState())
	{
		bool _alice = true;
		bool _bob = true;
		if (!(_alice=GenerateResult(_newposs_alice, &_newmoney_alice, Bob, Alice)) 
			|| !(_bob=GenerateResult(_newposs_bob,&_newmoney_bob, Alice, Bob)) )
		{
			if (!_alice && problem_roleid != NULL) 
				*problem_roleid = Alice->roleid;
			if (!_bob && problem_roleid != NULL) 
				*problem_roleid = Bob->roleid;
			blGenResult = false;

			//change state to exchange, let traders to adjust their objects
			ChangeState(&_TState_Exchange);

			if (Alice->GetState() == Trader::READY) 
				Alice->ChangeState(Trader::EXCHANGE);
			if (Bob->GetState() == Trader::READY) 
				Bob->ChangeState(Trader::EXCHANGE);

			return ERR_TRADE_SPACE;	//insufficient space
		}
		else
		{
			blGenResult = true;	//generation of transaction result is done!
			ChangeState(&_TState_Ready);
			return ERR_TRADE_READY;
		}
	}
	else
	{
		ChangeState(&_TState_Ready_half);
		return ERR_TRADE_READY_HALF;
	}
}
int Transaction::Confirm(int roleid,int* problem_roleid)
{
	Thread::Mutex::Scoped l(locker_self);
	Trader* t= FindTrader(roleid);
	if (t==NULL) return ERR_TRADE_INVALID_TRADER;
	if (!state->OptionPolicy(Trader::OPT_CONFIRM))
		return ERR_TRADE_CONFIRM_FAIL;
	if (!t->Confirm()) return ERR_TRADE_CONFIRM_FAIL;
	if (Alice->GetState() == Bob->GetState())
	{
		ChangeState(&_TState_Confirm);
		DEBUG_PRINT("Transaction Done. alice state=%d, bob state=%d(id:%d)\n",
			Alice->GetState(),Bob->GetState(),roleid);
		if (Alice->exchg_money || Alice->exchg_obj.size() || Bob->exchg_money || Bob->exchg_obj.size())
		{
			if (!blGenResult)
			{
				TradeItems _pssn_A,_pssn_B;
				unsigned int _money_A,_money_B;
				if (!GenerateResult(_pssn_A,&_money_A,Bob,Alice)) return ERR_TRADE_SPACE;
				if (!GenerateResult(_pssn_B,&_money_B,Alice,Bob)) return ERR_TRADE_SPACE;
				Alice->possession.swap(_pssn_A);
				Alice->pssn_money=_money_A;
				Bob->possession.swap(_pssn_B);
				Bob->pssn_money=_money_B;
			}
			else
			{
				Alice->possession.swap(_newposs_alice);
				Alice->pssn_money=_newmoney_alice;
				Bob->possession.swap(_newposs_bob);
				Bob->pssn_money=_newmoney_bob;
			}
			Alice->pssn_modified = true;
			Bob->pssn_modified = true;
			LogTrade();
		}
		return ERR_TRADE_DONE;
	}
	else
	{
		ChangeState(&_TState_Confirm_half);
		DEBUG_PRINT("Confirm half. alice state=%d, bob state=%d(id:%d)\n",
			Alice->GetState(),Bob->GetState(),roleid);
		return ERR_TRADE_HALFDONE;
	}
}
int Transaction::Discard()
{
	Thread::Mutex::Scoped l(locker_self);
	if (!state->OptionPolicy(Trader::OPT_DISCARD))
		return ERR_TRADE_DISCARDFAIL;

	TradeItems _pssn;
	unsigned int _money;
	GenerateResult(_pssn, &_money, Alice, Alice);
	Alice->possession.swap(_pssn); Alice->pssn_money = _money;
	_pssn.clear();
	GenerateResult(_pssn, &_money, Bob, Bob);
	Bob->possession.swap(_pssn); Bob->pssn_money = _money;

	ChangeState(&_TState_Init);
	//Alice and Bob's pssn_modified is decided only by whether they moved objects
	return ERR_SUCCESS;
}
bool Transaction::GetExchgResult(TradeItems &exchg1, unsigned int* money1, 
		TradeItems &exchg2, unsigned int *money2, bool &blNeedSave)
{
	Thread::Mutex::Scoped l(locker_self);
	if (Alice==NULL || Bob==NULL) return false;

	exchg1.swap(Alice->possession);
	*money1=Alice->pssn_money;

	exchg2.swap(Bob->possession);
	*money2=Bob->pssn_money;	
	blNeedSave=(Alice->pssn_modified || Bob->pssn_modified);
	return true;
}
void Transaction::Destroy(bool blRollback)
{
	Thread::Mutex::Scoped l(locker_self);
	if (blRollback)
	{
		if (Alice) Alice->pssn_modified=false;
		if (Bob) Bob->pssn_modified=false;
	}
	ChangeState(&_TState_Null);
}
bool Transaction::GenerateResult(TradeItems &possession, unsigned int *money, Trader *from, Trader *to)
{
	if ( to == NULL || from == NULL ) 
		return false;

	//check money
	if ( to->pssn_money + from->exchg_money > 200000000 )
		return false;

	*money = to->pssn_money + from->exchg_money;
	return to->MergeInventory(from->exchg_obj, possession);
}

void Transaction::LogTrade()
{
	Log::ObjectVector AliceExgObjs,BobExgObjs;
	for (size_t i=0;i<Alice->exchg_obj.size();i++)
		AliceExgObjs.push_back( 
				Log::trade_object(Alice->exchg_obj[i].id,Alice->exchg_obj[i].pos,Alice->exchg_obj[i].count) 
				);
	for (size_t i=0;i<Bob->exchg_obj.size();i++)
		BobExgObjs.push_back( 
				Log::trade_object(Bob->exchg_obj[i].id,Bob->exchg_obj[i].pos,Bob->exchg_obj[i].count) 
				);
	Log::trade(Alice->roleid,Bob->roleid,Alice->exchg_money,Bob->exchg_money, AliceExgObjs,BobExgObjs);
}
void Transaction::ShowTrader()
{
	if (Alice) Alice->Show();
	if (Bob) Bob->Show();
}

void Transaction::DiscardTransaction(int roleid)
{
	if(!roleid)
		return;
	GNET::Transaction* t;
	if ((t=Transaction::FindTransactionbyTrader(roleid)))
	{
		if(t->Discard()==ERR_SUCCESS )
		{
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			Trader* id;
			if((id=t->GetAlice()))
				dsm->Send(id->linksid,TradeDiscard_Re(ERR_SUCCESS,t->GetTid(),roleid,id->roleid,id->localsid));
			if((id=t->GetBob()))
				dsm->Send(id->linksid,TradeDiscard_Re(ERR_SUCCESS,t->GetTid(),roleid,id->roleid,id->localsid));
			t->Destroy(true);
		}
	}
}

} ;// namespace GNET
