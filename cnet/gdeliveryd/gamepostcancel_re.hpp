
#ifndef __GNET_GAMEPOSTCANCEL_RE_HPP
#define __GNET_GAMEPOSTCANCEL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GamePostCancel_Re : public GNET::Protocol
{
	#include "gamepostcancel_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("gamepostcancel_re: receive. retcode=%d roleid=%lld,sn=%lld\n",retcode,roleid,sn);
/*
gamepostcancel_re
0.成功； 
1.sn不存在（gs认为成功，不重发）； 
2.重复取消寄售（gs认为成功，不重发）；
3.其他错误（重发）； 
4. 往平台发送数据失败（重发）； 
5.平台根据sn查到的roleid与游戏服务器指定的roleid不等（gs和平台忽略该错误，gs认为成功，不重发）；
6.物品当前不能取消寄售，gs收到后将物品状态改为前一状态（一般是平台做了好几个操作都没通知到游戏，此时以平台端物品状态为准，物品在平台端可能是在架/公示/售出， 不再重发）； 
7．平台根据sn查到的userid与游戏指定的userid不一致（gs和平台忽略该错误，gs认为成功，不重发）；
-1. 平台数据库错误（重发）；
*/
		WebTradeMarket& market = WebTradeMarket::GetInstance();
		switch(retcode)
		{
			case 0:
			case 1:
			case 2:
			case 5:
			case 7:
				market.RecvCancelPostRe(true,userid,sn);
				break;
			case 6:
				market.RecvCancelPostRe(false,userid,sn);
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
