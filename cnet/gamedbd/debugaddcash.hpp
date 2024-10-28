
#ifndef __GNET_DEBUGADDCASH_HPP
#define __GNET_DEBUGADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gcashvipinfo"


namespace GNET
{

class DebugAddCash : public GNET::Protocol
{
	#include "debugaddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		AddCash_Re re(0, userid, 0, 0);
		try
		{
			StorageEnv::Storage * puser = StorageEnv::GetStorage("user");
			StorageEnv::CommonTransaction txn;
			Marshal::OctetsStream key;
			User user;
			try
			{
				key << (unsigned int)userid;
				Marshal::OctetsStream(puser->find(key,txn))>>user;
				if(user.cash_used > 2000000000)	//gs debug加元宝命令会减少cash_used，导致该值接近unsigned int上限
				{
					user.cash_buy = 0;
					user.cash_sell = 0;
					user.cash_used = 0;
					user.cash_add = 0;
					user.cash = 0;
					user.cash_sysauction.clear();
				}
				user.cash_add += cash;
				LOG_TRACE("debugaddcash userid=%d:cash_add=%d:cash_used=%d:cash_buy=%d:cash_sell=%d", userid, user.cash_add, user.cash_used, user.cash_buy, user.cash_sell);
				/* 
				 CASH VIP
				*/
				GRewardStore reward_store;
				if(0 != user.consume_reward.size())
				{
					Marshal::OctetsStream os_reward(user.consume_reward);
					os_reward >> reward_store;
				}
				GCashVipInfo vipinfo;
				if(0 != reward_store.cash_vip_info.size())
				{
					Marshal::OctetsStream os_vip_info(reward_store.cash_vip_info);
					try
					{
						os_vip_info >> vipinfo;
					}
					catch( ... )
					{
						vipinfo.purchase_limit_items_info = Octets(0);
					}
				}
				if(vipinfo.cash_vip_level == 0)
					vipinfo.score_cost_stamp = CashVip::GetTodayReduceScoreStamp();
				vipinfo.score_add += (cash / 100 * 3);
				int cur_score = vipinfo.score_add - vipinfo.score_cost - vipinfo.score_consume;
				CashVip::CalCashVipLevel(vipinfo.cash_vip_level, cur_score);
				
				reward_store.cash_vip_info = Marshal::OctetsStream() << vipinfo;
				user.consume_reward = Marshal::OctetsStream() << reward_store;

				re.cash_stub = user.cash_add;
				puser->insert( key, Marshal::OctetsStream()<<user, txn );
				re.retcode = ERR_SUCCESS;
			}
			catch ( DbException e ) { throw; }
			catch ( ... )
			{
				DbException ee( DB_OLD_VERSION );
				txn.abort( ee );
				throw ee;
			}
		}
		catch ( DbException e )
		{
			Log::log( LOG_ERR, "DebugAddCash, userid=%d, what=%s\n", userid, e.what() );
			re.retcode = CASH_ADD_FAILED;
		}
		manager->Send(sid, re);
	}
};

};

#endif
