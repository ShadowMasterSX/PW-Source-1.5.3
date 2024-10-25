#include "types.h"
#include "obj_interface.h"
#include "sysauctionlib.h"
#include "libcommon.h"

#include "gproviderclient.hpp"

#include "sysauctionlist.hpp"
#include "sysauctiongetitem.hpp"
#include "sysauctionaccount.hpp"
#include "sysauctionbid.hpp"
#include "sysauctioncashtransfer.hpp"


#define GDELIVERY_SERVER_ID  0

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

bool Handle_SysAuctionList(SysAuctionList& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_SysAuctionGetItem(SysAuctionGetItem& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_SysAuctionAccount(SysAuctionAccount& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_SysAuctionBid(SysAuctionBid& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_SysAuctionCashTransfer(SysAuctionCashTransfer& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	GMailSyncData syncdata;
	if( !GetSyncData(syncdata,obj_if) )
		return false;
	proto.localsid = obj_if.GetLinkSID();
	Marshal::OctetsStream os;
	os << syncdata;
	proto.syncdata = os;
	if(obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if( GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto ) )
			return true;
		obj_if.TradeUnLockPlayer();		
	}
	return false;	
}

bool ForwardSysAuctionOP(unsigned int type, const void * pParams, size_t param_len,object_interface obj_if)	
{
	try {
		Marshal::OctetsStream os( Octets(pParams,param_len) );
		switch ( type )
		{
			CASE_PROTO_HANDLE(SysAuctionList)
			CASE_PROTO_HANDLE(SysAuctionGetItem)
			CASE_PROTO_HANDLE(SysAuctionAccount)
			CASE_PROTO_HANDLE(SysAuctionBid)
			CASE_PROTO_HANDLE(SysAuctionCashTransfer)
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
