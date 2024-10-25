
#ifndef __GNET_POST_RE_HPP
#define __GNET_POST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "timeinfo"

namespace GNET
{

class Post_Re : public GNET::Protocol
{
	#include "post_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("post_re: receive. retcode=%d roleid=%lld,sn=%lld showperiod=%d postperiod=%d commodity_id=%d\n",retcode,roleid,sn,time.showperiod,time.postperiod,commodity_id);
/*
post_re
0.成功；
1.同一sn重复寄售(gs认为是寄售成功)； 
2.密保等级不够，不能寄售（gs认为是寄售失败，不再重发）； 
3.其他错误（重发）； 
4.往平台发送数据失败（重发）； 
5.物品不存在（平台根据xml查不到该物品，gs收到该错误码认为寄售失败，不重发）；
6.因为回档造成的sn冲突（gs认为寄售失败，不重发）； 
7.帐号被封，禁止寄售物品（gs认为寄售失败，不重发）；
8.该服务器暂时禁止寄售（gs认为寄售失败，不重发）；
9.服务器非法，禁止寄售（gs认为寄售失败，不重发）；
-1.平台数据库错误（重发）；
*/
		WebTradeMarket& market = WebTradeMarket::GetInstance();		
		switch(retcode)
		{
			case 0:
			case 1:
				market.RecvPostRe(true,userid, sn, time.postperiod, time.showperiod, commodity_id);
				break;
			case 6:
				market.AdvanceSN();
			case 2:
			case 5:
			case 7://也许需要增加提示给客户端
			case 8:
			case 9:
				market.RecvPostRe(false,userid, sn,0,0,0);
				break;
			case 3:	
			case 4:
			case -1:
			default:
				break;
		}
	}
};

};

#endif
