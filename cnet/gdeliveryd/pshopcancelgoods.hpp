#ifndef __GNET_PSHOPCANCELGOODS_HPP
#define __GNET_PSHOPCANCELGOODS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "pshopcancelgoods_re.hpp"
#include "dbpshopcancelgoods.hrp"

namespace GNET
{

class PShopCancelGoods : public GNET::Protocol
{
	#include "pshopcancelgoods"

	void SendErr(const PlayerInfo & pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo.linksid, PShopCancelGoods_Re(retcode, pinfo.localsid, canceltype, pos));
	}
	bool QueryDB(const PlayerInfo & pinfo) const
	{
		DBPShopCancelGoodsArg arg(roleid, canceltype, pos);
		DBPShopCancelGoods *rpc = (DBPShopCancelGoods *)Rpc::Call(RPC_DBPSHOPCANCELGOODS, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	int CheckCondition(const PShopObj *obj) const
	{
		if(obj->IsBusy())
			return ERR_PLAYERSHOP_BUSY;
		if(canceltype == DBPShopCancelGoods::TYPE_CANCEL_BUY && !(obj->GetItemBuy(pos)))
			return ERR_PLAYERSHOP_ITEM_NOTFOUND;
		else if(canceltype == DBPShopCancelGoods::TYPE_CANCEL_SALE)
		{
			if(!(obj->GetItemSale(pos)))//此栏位无物品
				return ERR_PLAYERSHOP_ITEM_NOTFOUND;
			if(PSHOP_STORE_CAP - obj->GetStore().size() <= obj->GetListBuy().size())
			{
				//取消收购物品需要占用仓库栏位
				//所以这里需要保证店铺收购不受影响
				return ERR_PLAYERSHOP_STORE_FULL;
			}
		}

		return ERR_SUCCESS;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(*pinfo, ERR_SHOPMARKET_NOT_INIT);
			return;
		}
		
		if(pos < 0 || canceltype < 0 || canceltype > 1) return;

		PShopObj *obj = PShopMarket::GetInstance().GetShop(roleid);
		if(obj)
		{
			int retcode = CheckCondition(obj);
			if(retcode != ERR_SUCCESS)
			{
				SendErr(*pinfo, retcode);
				return;
			}
		}
		else
		{
			//obj为NULL时店铺可能已过期
			//店铺过期状态也支持取消操作
		}

		if(!QueryDB(*pinfo))
		{
			SendErr(*pinfo, ERR_FAILED);
			return;
		}

		if(obj) obj->SetBusy(true);
	}
};

};

#endif
