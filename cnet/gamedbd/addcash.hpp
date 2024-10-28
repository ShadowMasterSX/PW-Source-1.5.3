
#ifndef __GNET_ADDCASH_HPP
#define __GNET_ADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "user"
#include "addcash_re.hpp"
#include "localmacro.h"
#include "gsysauctioncash"
#include "gcashvipinfo"

#include "gamedbmanager.h"

namespace GNET
{

class AddCash : public GNET::Protocol
{
	#include "addcash"
	
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		AddCash_Re re(0, userid, zoneid, sn);
		try
		{
			StorageEnv::Storage * puser = StorageEnv::GetStorage("user");
			StorageEnv::CommonTransaction txn;
			Marshal::OctetsStream key;
			User user;
			GSysAuctionCash sa_cash;
			try
			{
				re.zoneid = zoneid;
				re.sn = sn;

				key << (unsigned int)userid;
				Marshal::OctetsStream(puser->find(key,txn))>>user;
				if(user.cash_sysauction.size())
					Marshal::OctetsStream(user.cash_sysauction)>>sa_cash;
				if(user.add_serial<sn)
				{
					int total = user.cash_add + user.cash_buy - user.cash_used - user.cash_sell - sa_cash.cash_used_2;
					if(cash<0 && total+cash<0)
						re.retcode = CASH_NOT_ENOUGH;
					else
					{
						int used = 0;
						if((int)(user.cash_add+cash)<0)
						{
							if(!GameDBManager::GetInstance()->IsJapanVersion())
							{
								Log::log( LOG_ERR, "AddCash, userid=%d, int overflow cash=%d delta=%d",
										userid, user.cash_add, cash);
								throw DbException(DB_VERIFY_BAD);
							}
							else
							{
								used = - cash - user.cash_add;
								cash = - user.cash_add;
							}
						}
						user.add_serial = sn;
						user.cash_add += cash;
						Log::formatlog("addcash","userid=%d:oldserial=%d:newserial=%d:cash_add=%d:delta=%d",
							userid,user.add_serial,sn,user.cash_add, cash);
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
							os_vip_info >> vipinfo;
						}
						if(vipinfo.cash_vip_level == 0)
							vipinfo.score_cost_stamp = CashVip::GetTodayReduceScoreStamp();
						vipinfo.score_add += (cash / 100 * 3);
						int cur_score = vipinfo.score_add - vipinfo.score_cost - vipinfo.score_consume;
						CashVip::CalCashVipLevel(vipinfo.cash_vip_level, cur_score);
						Log::formatlog("CASH_VIP ADD_CASH","userid=%d:oldserial=%d:newserial=%d:score_add=%d:vip_level=%d:score_daily_reduce=%d:score_consume=%d",
							userid,user.add_serial,sn,vipinfo.score_add, vipinfo.cash_vip_level, vipinfo.score_cost, vipinfo.score_consume);
						
						reward_store.cash_vip_info = Marshal::OctetsStream() << vipinfo;
						user.consume_reward = Marshal::OctetsStream() << reward_store;
						
						if(used)
						{
							/*
							 由于此处修改了cash_used，所以玩家当前不能在线 
							 */
							user.cash_used += used;
							struct
							{
								int id;
								int expire;
								int guid1;
								int guid2;
							}dummy_item = {1, 0, 0, 0};	//伪造的商城物品，价格为1。
							int pocket_cash = user.cash_add+user.cash_buy-user.cash_sell-user.cash_used-sa_cash.cash_used_2-user.cash-sa_cash.cash_2; //GS商城中看到的剩余值
							Log::formatlog("gshop_trade","userid=%d:db_magic_number=%d:order_id=%d:item_id=%d:expire=%d:item_count=%d:cash_need=%d:cash_left=%d:guid1=%d:guid2=%d",
									0/*无效roleid*/, userid, user.use_serial, dummy_item.id, dummy_item.expire, used, used, pocket_cash, dummy_item.guid1, dummy_item.guid2);
							user.use_serial ++;
						}						
						re.cash_stub = user.cash_add;
						puser->insert( key, Marshal::OctetsStream()<<user, txn );
						re.retcode = ERR_SUCCESS;
					}
				}else
				{
					Log::formatlog("addok","userid=%d:old_serial=%d:new_serial=%d:cash_add=%d:delta=%d",
						userid, user.add_serial, sn, user.cash_add, cash);
				}
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
			Log::log( LOG_ERR, "AddCash, userid=%d, what=%s\n", userid, e.what() );
			re.retcode = CASH_ADD_FAILED;
		}
		manager->Send(sid, re);
	}
};

};

#endif
