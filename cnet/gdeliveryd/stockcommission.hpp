
#ifndef __GNET_STOCKCOMMISSION_HPP
#define __GNET_STOCKCOMMISSION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "stockexchange.h"
#include "dbstockcommission.hrp"
#include "stockcommission_re.hpp"

namespace GNET
{

class StockCommission : public GNET::Protocol
{
	#include "stockcommission"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("StockCommission, roleid=%d,isbuy=%d,price=%d,volume=%d",roleid,isbuy,price,volume);
                PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
                if(!pinfo)
                        return;
                if(isbuy)
                        price = -price;
                unsigned int tid;
                int ack_volume;
                int ret = StockExchange::Instance()->PlaceOrder(pinfo->userid,price,volume,ack_volume,tid);
                if(ret)
                {
                        DEBUG_PRINT("StockCommission, tid=%d, ret=%d",tid, ret);
                        StockCommission_Re res;
                        res.retcode = ret;
                        res.localsid = pinfo->localsid;
                        GDeliveryServer::GetInstance()->Send(pinfo->linksid, res);
                }
                else
                {
                        DEBUG_PRINT("StockCommission, tid=%d, ack_volume=%d",tid, ack_volume);
                        DBStockCommissionArg arg;
                        arg.order.tid = tid;
                        arg.order.time = Timer::GetTime();
                        arg.order.userid = pinfo->userid;
                        arg.order.price = price;
                        arg.order.volume = volume;
                        arg.order.status = 0;

                        DBStockCommission* rpc = (DBStockCommission*) Rpc::Call( RPC_DBSTOCKCOMMISSION,arg);
                        rpc->linksid = pinfo->linksid;
                        rpc->localsid = pinfo->localsid;
                        rpc->ack_volume = ack_volume;
                        GameDBClient::GetInstance()->SendProtocol(rpc);
                }
	}
};

};

#endif
