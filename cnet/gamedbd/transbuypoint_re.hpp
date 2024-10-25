
#ifndef __GNET_TRANSBUYPOINT_RE_HPP
#define __GNET_TRANSBUYPOINT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "transid"
#include "sellid"
#include "sellpointinfo"
#include "transman.h"
#include "selldataimage.h"
#include "syncsellinfo.hpp"
#include "gamedbserver.hpp"
namespace GNET
{

class TransBuyPoint_Re : public GNET::Protocol
{
	#include "transbuypoint_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Log::formatlog("sellinfo","TransBuyPoint_Re. ret=%d,transid(z:%d,s:%d),sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,aid=%d\n", retcode,tid.zoneid,tid.serialno,sellid.roleid,sellid.sellid,buyer,price,point,aid);
		Thread::Mutex::Scoped l(TransMan::GetInstance().GetLock());
		try 
		{
			StorageEnv::Storage * ptrans = StorageEnv::GetStorage("translog");
			StorageEnv::Storage * psell = StorageEnv::GetStorage("sellpoint");
			StorageEnv::CommonTransaction txn;
			//update sellinfo
			SellPointInfo sellinfo;
			Marshal::OctetsStream( psell->find( Marshal::OctetsStream()<<sellid,txn ) ) >> sellinfo;
			if ( retcode==ERR_SP_ABORT || retcode==ERR_SP_EXCESS )
			{
				sellinfo.status = _PST_NOTSELL;
				psell->insert( 
					Marshal::OctetsStream()<<sellid,
					Marshal::OctetsStream()<<sellinfo,
					txn
				);	
			}
			else if ( retcode==ERR_SP_COMMIT )
			{
				sellinfo.status = _PST_SOLD;
				psell->del( 
					Marshal::OctetsStream()<<sellid,
					txn
				);	
			}
			else
			{
				return;
			}
			//update trans status
			int trans_status= (retcode==ERR_SP_ABORT) ? 
							  SellRecord::value_type::_ST_ABORT : SellRecord::value_type::_ST_COMMIT;
			int ret=TransMan::GetInstance().UpdateTrans_NoLock(
					TransMan::TransKey(tid.zoneid,tid.serialno),
					trans_status,
					ptrans,
					txn
				);
			if ( ret==ERR_SUCCESS )
			{
				if ( sellinfo.status==_PST_SOLD )
					SellDataImage::GetInstance().OnRmvSell(sellid.roleid,sellid.sellid);
				GameDBServer::GetInstance()->Send2Delivery( SyncSellInfo(sellinfo,buyer) );
			}
			else
			{
				Log::log(LOG_ERR,"TransBuyPoint_Re. ret=%d,transid(z:%d,s:%d),sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,aid=%d\n",
					ret,tid.zoneid,tid.serialno,sellid.roleid,sellid.sellid,buyer,price,point,aid);
			}
		}
		catch (DbException e )
		{
			Log::log(LOG_ERR,"TransBuyPoint_Re. failed.what=%s,transid(z:%d,s:%d),sellid(r:%d,s:%d),buyer=%d,price=%d,point=%d,aid=%d\n",e.what(),tid.zoneid,tid.serialno,sellid.roleid,sellid.sellid,buyer,price,point,aid);
		}

	}
};

};

#endif
