#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>
#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "cooldowncfg.h"

#include "touchtrade.h"
#include <gsp_if.h>
#include <mtime.h>

void touch_trade::OnHeartbeat(gplayer_imp * pImp)
{
	if(IsFree()) return;

	if(--_timeout <= 0)
	{
		switch(data.state)
		{
			case TSTATE_HALFTRADE:
				OnHalfCost(pImp);				
				break;
			case TSTATE_WAITPOCKET:
				OnWaitPocket(pImp);
				break;
			default:
				{
					GLog::log(LOG_ERR,"roleid:%d touch sn %lld state%d error[item%d count%d expire%d lots%d cost%d index%d] income%lld remain%lld",
					_roleid, data.sn, data.state, data.itemtype, data.itemcount, data.itemexpire, data.lots, data.cost, _itemindex, income, remain);
					ClearData();
				}
				break;
		}

	}
		
}

bool touch_trade::Query(gplayer_imp * pImp)
{
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_TOUCHTRADE) )
	{
		return false;
	}
	
	GMSV::SendPlayerQueryTouchPoint(_roleid);

	pImp->SetCoolDown(COOLDOWN_INDEX_TOUCHTRADE,TOUCH_QUERY_COOLDOWN_TIME);

	return true;
}

bool touch_trade::TryCost(gplayer_imp * pImp,unsigned int index,int type,unsigned int count ,unsigned int price,int expire,unsigned int lots)
{
	ASSERT(IsFree());
	
	if(!pImp->CheckCoolDown(COOLDOWN_INDEX_TOUCHTRADE) )
	{
		return false;
	}

	int64_t newsn =	MakeNewSN(_roleid);

	if(newsn < data.sn)
	{
		// log
		GLog::log(LOG_ERR,"roleid:%d old %lld new %lld sn make warning",_roleid,data.sn,newsn);
		newsn = data.sn + 0x100000000LL; // add 1 sec to old sn
	}

	data.sn = newsn;
	data.itemtype = type;
	data.itemcount = count;
	data.itemexpire = expire;
	data.cost = price;
	data.lots = lots;
	data.state = TSTATE_HALFTRADE;
	
	_itemindex = index;

	GMSV::SendPlayerCostTouchPoint(_roleid,data.sn,data.cost);

	SetTimeOut(TOUCH_CAST_WAIT_TIME);

	pImp->SetCoolDown(COOLDOWN_INDEX_TOUCHTRADE,TOUCH_QUERY_COOLDOWN_TIME);
	return true;
}

bool touch_trade::OnIdClash(gplayer_imp * pImp)
{
	GLog::log(LOG_ERR,"roleid:%d touch orderid%lld clash",_roleid,data.sn);
	
	pImp->_runner->notify_touch_cost(income,remain,data.cost,_itemindex,data.lots,TPC_ORDER_CLASH);

	ClearData();

	return true;
}

void touch_trade::ClearData()
{
	data.state = TSTATE_FREE;
	data.sn = 0;
	data.cost = 0;
	data.lots = 0;
	data.itemtype = 0;
	data.itemcount = 0;
	data.itemexpire = 0;
	
	_itemindex = 0;
	_noticepass = 0;	
}

int64_t touch_trade::MakeNewSN(int roleid)
{
	int64_t newsn;
	int* pw = (int*)&newsn;

	*pw = roleid;	
	*(++pw) = g_timer.get_systime();

	return newsn;
}

bool touch_trade::Complete(gplayer_imp * pImp)
{
	int res = TPC_SUCCESS;
	int pile_limit = world_manager::GetDataMan().get_item_pile_limit(data.itemtype);
	if( 0 >= pile_limit)
	{
		return false;
	}

	unsigned int needslots = ((data.itemcount*data.lots + pile_limit - 1)/pile_limit);	

	if(!pImp->InventoryHasSlot(needslots) || pImp->_lock_inventory || 
	   (pImp->GetPlayerState() != gplayer_imp::PLAYER_STATE_NORMAL && pImp->GetPlayerState() != gplayer_imp::PLAYER_SIT_DOWN))
	{
		data.state = TSTATE_WAITPOCKET;
		SetTimeOut(TOUCH_POCKET_WAIT_TIME);
		res = TPC_NEED_SLOT;
		if(!(_noticepass++ % NOTICE_INTERVAL))  // 背包条件不足间隔通知客户端
		{
			pImp->_runner->notify_touch_cost(income,remain,data.cost,_itemindex,data.lots,res);
		}
	}
	else
	{
		GLog::log(LOG_INFO,"roleid:%d touch sn %lld complate data[item%d count%d expire%d lots%d cost%d index%d] income%lld remain%lld",
			_roleid, data.sn, data.itemtype, data.itemcount, data.itemexpire, data.lots, data.cost, _itemindex, income, remain);
		
		PurchaseItem(pImp,data.itemtype,data.itemcount,data.itemexpire,data.lots);
		pImp->_runner->notify_touch_cost(income,remain,data.cost,_itemindex,data.lots,res);
		ClearData();
	}

	return res == TPC_SUCCESS;
}

void touch_trade::OnHalfCost(gplayer_imp * pImp)
{
	GLog::log(LOG_INFO,"roleid:%d touch sn %lld halfcost tick",_roleid,data.sn);
	GMSV::SendPlayerCostTouchPoint(_roleid,data.sn,data.cost);
	SetTimeOut(TOUCH_CAST_WAIT_TIME);
}

void touch_trade::OnWaitPocket(gplayer_imp * pImp)
{
	GLog::log(LOG_INFO,"roleid:%d touch sn %lld waitpocket tick",_roleid,data.sn);
	Complete(pImp);	
}

void touch_trade::PurchaseItem(gplayer_imp * pImp,int type, int count,int expire,int lots) 
{
	const item_data * pItem = (const item_data*)world_manager::GetDataMan().get_item_for_sell(type);
	if(!pItem) return;

	item_data * pItem2 = DupeItem(*pItem);
	
	int guid1 = 0;
	int guid2 = 0;   
	if(pItem2->guid.guid1 != 0) 
	{ 
		 get_item_guid(type, guid1,guid2); 
		 pItem2->guid.guid1 = guid1;
		 pItem2->guid.guid2 = guid2; 
	} 
//	pItem2->proc_type |= item::ITEM_PROC_TYPE_TOUCH_MALL; 
	item_list & inventory = pImp->GetInventory();

	int pile_limit = pItem->pile_limit;
	int expire_date = expire ? g_timer.get_systime() + expire : 0; 
	unsigned int totalcount = count*lots;
	
	while(totalcount)
	{
		int ocount = 0;
		if(totalcount > (unsigned int)pile_limit)
		{
			ocount = pile_limit;
			totalcount -= pile_limit;
		}
		else
		{
			ocount = totalcount;
			totalcount = 0;
		}

		pItem2->count = ocount;
		pItem2->expire_date = expire_date;
		int rst = inventory.Push(*pItem2);
		ASSERT(rst >= 0 && pItem2->count == 0);
        inventory[rst].InitFromShop();

		pImp->_runner->obtain_item(pItem2->type,expire_date,ocount,inventory[rst].count, 0,rst);
	}
	GLog::log(GLOG_INFO,"用户%d从Touch商城购买了item%d count%dlots%d",pImp->_parent->ID.id, type, count,lots);
	
	FreeItem(pItem2);
}


