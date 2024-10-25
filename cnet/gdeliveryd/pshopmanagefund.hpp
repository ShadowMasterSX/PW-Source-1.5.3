#ifndef __GNET_PSHOPMANAGEFUND_HPP
#define __GNET_PSHOPMANAGEFUND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "pshopmanagefund_re.hpp"
#include "dbpshopmanagefund.hrp"

namespace GNET
{

class PShopManageFund : public GNET::Protocol
{
	#include "pshopmanagefund"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}

	void SendErr(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopManageFund_Re(retcode, pinfo->localsid));
	}

	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData &data) const
	{
		DBPShopManageFundArg arg(roleid, optype, money, yinpiao, data);
		DBPShopManageFund *rpc = (DBPShopManageFund *)Rpc::Call(RPC_DBPSHOPMANAGEFUND, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	bool CheckCondition(const PShopObj *obj)
	{
		/*
		 * 包裹检查在gamed端已完成
		 * 这里只进行店铺相关检查
		 * 取钱:
		 *      金钱不够则失败,不会用银票填充
		 *      且取钱不能印象收购栏的正常收购,否则失败
		 * 存钱:
		 *      需要保证店铺仓库有足够空间保存出售栏出售获得的金钱
		 */

		if(optype == 0)//存钱
		{
			if((PSHOP_MONEY_CAP - obj->GetMoney()) < money)
				return false;//金钱满
			if((PSHOP_YINPIAO_CAP - obj->GetYinPiao()) < yinpiao)
				return false;//银票满

			uint64_t money_save = (uint64_t)money + (uint64_t)yinpiao * (uint64_t)WANMEI_YINPIAO_PRICE;
			uint64_t total_item_value = 0;//出售栏价值
			const PShopItemVector &slist = obj->GetListSale();
			for(size_t i=0; i<slist.size(); ++i)
				total_item_value += ((uint64_t)slist[i].item.count * (uint64_t)slist[i].price);
			if((obj->GetTotalMoney() + total_item_value + money_save) > obj->GetTotalMoneyCap())
				return false;//金钱满
			return true;
		}
		else if(optype == 1)//取钱
		{
			if(obj->GetMoney() < money) return false;
			if(obj->GetYinPiao() < yinpiao) return false;

			uint64_t money_draw = (uint64_t)money + (uint64_t)yinpiao * (uint64_t)WANMEI_YINPIAO_PRICE;
			uint64_t total_cost = 0;//收购栏的消耗多少金钱
			const PShopItemVector &blist = obj->GetListBuy();
			for(size_t i=0; i<blist.size(); ++i)
				total_cost += ((uint64_t)blist[i].item.count * (uint64_t)blist[i].price);
			return (obj->GetTotalMoney() - total_cost) >= money_draw;

		}

		return true;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GMailSyncData data;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}
		catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::PShopManageFund: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(pinfo, ERR_SHOPMARKET_NOT_INIT);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}
		
		if(optype < 0 || (money <= 0 && yinpiao <= 0))
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		PShopObj *obj = PShopMarket::GetInstance().GetShop(roleid);
		if(obj)
		{
			if(!CheckCondition(obj))
			{
				SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
				return;
			}
		}
		else if(optype == 0)//存钱
		{
			//店铺不存在或已过期
			//过期状态不支持存钱操作
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			if(PShopMarket::GetInstance().GetFromTimeMap(roleid))
				SendErr(pinfo, ERR_PLAYERSHOP_EXPIRED);
			return;
		}

		if(!QueryDB(*pinfo, data))
		{
			SendErr(pinfo, ERR_FAILED);
			SyncGameServer(pinfo, data, ERR_FAILED);
		}
	}
};

};

#endif
