#include "types.h"
#include "obj_interface.h"
#include "webtradesyslib.h"
#include "libcommon.h"

#include "gproviderclient.hpp"

#include "webtradeprepost.hpp"
#include "webtradeprecancelpost.hpp"
#include "webtradelist.hpp"
#include "webtradegetitem.hpp"
#include "webtradegetdetail.hpp"
#include "webtradeattendlist.hpp"
#include "webtradeupdate.hpp"


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


bool Handle_WebTradePrePost( WebTradePrePost& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) 
		return false;
	switch(proto.posttype)
	{
		case 1:		//寄售游戏币
			if(proto.money == 0 || proto.money > obj_if.GetMoney()) return false;
			break;
		case 2:		//寄售物品
			if(proto.item_num <= 0 || !obj_if.CheckItem(proto.item_pos,proto.item_id,proto.item_num)) return false;
			break;
		default:
			return false;
	}

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

bool Handle_WebTradePreCancelPost( WebTradePreCancelPost& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_WebTradeList(WebTradeList & proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_WebTradeGetItem(WebTradeGetItem & proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_WebTradeGetDetail(WebTradeGetDetail & proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_WebTradeAttendList(WebTradeAttendList& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool Handle_WebTradeUpdate(WebTradeUpdate& proto, object_interface& obj_if )
{
	if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
	proto.localsid=obj_if.GetLinkSID();
	return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
}

bool ForwardWebTradeSysOP(unsigned int type, const void * pParams, size_t param_len,object_interface obj_if)	
{
	try {
		Marshal::OctetsStream os( Octets(pParams,param_len) );
		switch ( type )
		{
			CASE_PROTO_HANDLE(WebTradePrePost)
			CASE_PROTO_HANDLE(WebTradePreCancelPost)
			CASE_PROTO_HANDLE(WebTradeList)
			CASE_PROTO_HANDLE(WebTradeGetItem)
			CASE_PROTO_HANDLE(WebTradeAttendList)
			CASE_PROTO_HANDLE(WebTradeGetDetail)
			CASE_PROTO_HANDLE(WebTradeUpdate)
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
