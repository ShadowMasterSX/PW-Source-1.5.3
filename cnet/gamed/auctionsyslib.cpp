#include "types.h"
#include "obj_interface.h"
#include "auctionsyslib.h"
#include "libcommon.h"

#include "gmailsyncdata"
#include "gproviderclient.hpp"

#include "auctionattendlist.hpp"
#include "auctionbid.hpp"
#include "sendauctionbid.hpp"
#include "auctionclose.hpp"
#include "auctionget.hpp"
#include "auctionlist.hpp"
#include "auctionopen.hpp"
#include "auctionexitbid.hpp"
#include "auctiongetitem.hpp"
#include "../gdbclient/db_if.h"
#include "auctionlistupdate.hpp"

#define GDELIVERY_SERVER_ID  0

#define EIGHT_HOUR      28800
#define SIXTEEN_HOUR    57600
#define TWENTYFOUR_HOUR 86400

#define CASE_PROTO_HANDLE(_proto_name_)\
	case _proto_name_::PROTOCOL_TYPE:\
	{\
		_proto_name_ proto;\
		proto.unmarshal( os );\
		if ( proto.GetType()!=_proto_name_::PROTOCOL_TYPE || !proto.SizePolicy(os.size()) )\
			return false;\
		return Handle_##_proto_name_( proto,obj_if );\
	}

namespace GNET
{
	int GetDeposit( int elapse_time, size_t price)
	{
		float fee_ratio = 0.0f;
		int fee = 0;
		if ( elapse_time==EIGHT_HOUR )
			fee_ratio=0.1f;
		else if ( elapse_time==SIXTEEN_HOUR )
			fee_ratio=0.15f;
		else if ( elapse_time==TWENTYFOUR_HOUR )
			fee_ratio=0.2f;
		else
			return -1;
		fee = (int)(price*fee_ratio);
		if(fee<500)
			return 500;
		return fee;
	}

	bool Handle_AuctionAttendList( AuctionAttendList& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}

	bool Handle_AuctionBid( AuctionBid& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		SendAuctionBid request(proto.roleid, proto.auctionid, proto.bidprice, proto.bin);
		request.localsid = obj_if.GetLinkSID();

		if (!GetSyncData(request.syncdata,obj_if)) 
			return false;

		if(request.bidprice > request.syncdata.inventory.money)
			return false;
		if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if (GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,request ) )
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}
	bool Handle_AuctionClose( AuctionClose& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_AuctionGet( AuctionGet& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_AuctionList( AuctionList& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	
	}
	bool Handle_AuctionExitBid( AuctionExitBid& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_AuctionGetItem( AuctionGetItem& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}

	bool Handle_AuctionOpen( AuctionOpen& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		//注：由于GAuctionItem数量类型使用的是unsigned short，且无法改变，所以在此限定拍卖物品不能超过65535
		if ( proto.item_num<=0 || proto.item_num > 65535 || !obj_if.CheckItem( proto.item_pos,proto.item_id,proto.item_num) ) 
			return false;

		GMailSyncData syncdata;
		if ( !GetSyncData(syncdata,obj_if) ) 
			return false;
		proto.localsid = obj_if.GetLinkSID();
		size_t price = obj_if.QueryItemPrice(proto.item_pos, proto.item_id) * proto.item_num;
		if(price > MAX_CASH_IN_POCKET)
			return false;
		proto.deposit = GetDeposit(proto.elapse_time, price);
		if(proto.deposit<0 || proto.deposit > syncdata.inventory.money)
			return false;

		Marshal::OctetsStream os;
		os << syncdata;
		proto.syncdata = os;
		if(obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if ( GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto ) )
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}
	
	bool Handle_AuctionListUpdate( AuctionListUpdate& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}

	bool ForwardAuctionSysOP( unsigned int type,const void* pParams,size_t param_len,object_interface obj_if )
	{
		try {
			Marshal::OctetsStream os( Octets(pParams,param_len) );
			switch ( type )
			{
				CASE_PROTO_HANDLE(AuctionAttendList)
				CASE_PROTO_HANDLE(AuctionBid)
				CASE_PROTO_HANDLE(AuctionClose)
				CASE_PROTO_HANDLE(AuctionGet)
				CASE_PROTO_HANDLE(AuctionList)
				CASE_PROTO_HANDLE(AuctionOpen)
				CASE_PROTO_HANDLE(AuctionExitBid)
				CASE_PROTO_HANDLE(AuctionGetItem)
				CASE_PROTO_HANDLE(AuctionListUpdate)
				default:
					return false;		
			}
		}
		catch ( Marshal::Exception )
		{
			return false;
		}
	}
}
