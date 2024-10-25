
#ifndef __GNET_WEBPOSTCANCEL_HPP
#define __GNET_WEBPOSTCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webpostcancel_re.hpp"

#include "webtrademarket.h"
#include "gwebtradeclient.hpp"

namespace GNET
{

class WebPostCancel : public GNET::Protocol
{
	#include "webpostcancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webpostcancel: receive. roleid=%lld,sn=%lld,ctype=%d\n",roleid,sn,ctype);
/*
WebPostCancel_Re
0.成功；
1.sn不存在（平台认为该单子错误，删除该单子，不再重发）； 
2.重复取消寄售（平台认为成功，不再重发）； 
3.其他错误（重发）；
4.向游戏服务器发送数据失败（重发）； 
5.游戏端根据sn查到的roleid不存在（可能是角色已被GM删除或回档等原因，gs和平台忽略该错误，平台认为成功，不重发）；
6.物品在游戏内处于“与平台同步中”的状态（即游戏内物品处于“与平台同步中”的状态时，不再处理平台发来的逻辑，直接返回本错误码，直到收到平台返回的Re，平台收到该错误码后不作任何处理，重发）；
7．游戏根据sn查到的userid与平台指定的userid不一致（gs和平台忽略该错误，平台认为成功，不重发）；
8．游戏根据sn查到的roleid与平台指定的roleid不一致（gs和平台忽略该错误，平台认为成功，不重发）；
-1.游戏服务器处理失败（重发）；
*/
		int retcode = WebTradeMarket::GetInstance().DoWebPostCancel(userid,roleid,sn,ctype,messageid,timestamp);
		if(retcode != ERR_SUCCESS)
		{
			WebPostCancel_Re re;
			re.userid = userid;
			re.roleid = roleid;
			re.sn = sn;
			re.messageid = messageid;
			switch(retcode)
			{
				default:
				case ERR_WT_UNOPEN:
					re.retcode = -1;
					break;
				case ERR_WT_ENTRY_NOT_FOUND:
					re.retcode = 1; 
					break;
				case ERR_WT_ENTRY_IS_BUSY:
					re.retcode = 6;
					break;
				case ERR_WT_TIMESTAMP_MISMATCH:	//包括重复取消寄售的情况
					re.retcode = 0;
					break;
			}
			GWebTradeClient::GetInstance()->SendProtocol(re);
		}
	}
};

};

#endif
