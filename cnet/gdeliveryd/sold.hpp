
#ifndef __GNET_SOLD_HPP
#define __GNET_SOLD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class Sold : public GNET::Protocol
{
	#include "sold"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sold: receive. zoneid=%d,roleid=%lld,sn=%lld,buyerroleid=%d,orderid=%d\n",zoneid,sellerroleid,sn,buyerroleid,orderid);
/*
Sold_Re
0.成功； 
1.sn不存在（平台退款）；
2.物品已卖出（平台认为成功，不再重发）； 
3.其他错误（重发）
4.找不到买家（平台退款）； 
5. 向游戏服务器发送数据失败（重发）； 
6. 游戏端根据sn查到的sellerroleid不存在（可能是角色已删除或是回档期间建立的角色，gs和平台忽略该错误，平台认为成功，不重发）；
7.物品在游戏内处于“与平台同步中”的状态 （即游戏内物品处于“与平台同步中”的状态时，不再处理平台发来的逻辑，直接返回本错误码，直到收到平台返回的Re，平台收到该错误码后不作任何处理，重发）；
8．游戏根据sn查到的selleruserid与平台指定的selleruserid不一致（暂时忽略该错误，gs和平台忽略该错误，平台认为成功，不重发）；
9．游戏根据sn查到的sellerroleid与平台指定的sellerroleid不一致（暂时忽略该错误，gs和平台忽略该错误，平台认为成功，不重发）；
10.协议中发送的buyerrolied与buyeruserid不匹配（很大可能是合服过程中买家roleid发生变化，平台认为成功，不重发，记log）；
11 是邮箱已满
12 超过帐号容纳角色限制
13 角色交易指定买家找不到，平台退款，继续交易
14 角色交易卖的角色找不到，平台退款，物品废除
15 不符合出售条件，退款
-1. 游戏服务器处理失败（重发）；
*/
		int retcode = WebTradeMarket::GetInstance().DoSold(zoneid, selleruserid, sellerroleid, sn, buyeruserid, buyerroleid, orderid, stype, timestamp);
		if(retcode != ERR_SUCCESS)
		{
			Sold_Re re;
			re.zoneid = zoneid;
			re.selleruserid = selleruserid;
			re.sellerroleid = sellerroleid;
			re.buyeruserid = buyeruserid;
			re.buyerroleid = buyerroleid;
			re.sn = sn;
			re.orderid = orderid;
			switch(retcode)
			{
				default:
				case ERR_WT_UNOPEN:
				case ERR_WT_BUYER_STATUS_INAPPROPRIATE:
					re.retcode = -1;
					break;
				case ERR_WT_ENTRY_HAS_BEEN_SOLD:
					re.retcode = 2;
					break;
				case ERR_WT_ENTRY_NOT_FOUND:
					re.retcode = 1; 
					break;
				case ERR_WT_ENTRY_IS_BUSY:
					re.retcode = 7; 
					break;
				case ERR_WT_TIMESTAMP_MISMATCH:
					re.retcode = 0;
					break;
				case ERR_WT_BUYER_NOT_EXIST:
					re.retcode = 4;
					break;
			}
			GWebTradeClient::GetInstance()->SendProtocol(re);
		}
	}
};

};

#endif
