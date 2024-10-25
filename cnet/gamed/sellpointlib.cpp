#include "types.h"
#include "obj_interface.h"
#include "sellpointlib.h"
#include "libcommon.h"

#include "gproviderclient.hpp"
//protocols
#include "dbsellpoint.hrp"
#include "dbbuypoint.hrp"
#include "sellpoint.hpp"
#include "getselllist.hpp"
#include "sellcancel.hpp"
#include "buypoint.hpp"

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
	bool Handle_SellPoint( SellPoint& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		//send dbsellpoint to delivery
		DBSellPoint* rpc=(DBSellPoint*) Rpc::Call(
				RPC_DBSELLPOINT,
				SellPointArg(
					proto.roleid,
					proto.localsid,
					proto.point,
					proto.price,
					obj_if.InceaseDBTimeStamp(),
					obj_if.GetMoney()
				)
			);
		if ( obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT)==0 )
		{
			if ( GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,rpc ) )
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}
	bool Handle_SellCancel( SellCancel& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_GetSellList( GetSellList& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		proto.localsid=obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,proto );
	}
	bool Handle_BuyPoint( BuyPoint& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) return false;
		//send dbbuypoint to gamedbd
		DBBuyPoint* rpc=(DBBuyPoint*) Rpc::Call(
				RPC_DBBUYPOINT,
				DBBuyPointArg(
					proto.roleid,
					obj_if.GetLinkSID(),
					proto.sellid,
					proto.seller,
					obj_if.InceaseDBTimeStamp(),
					obj_if.GetMoney()
				)
			);
		if ( obj_if.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT)==0 )
		{
			if ( GProviderClient::GetInstance()->DispatchProtocol( GDELIVERY_SERVER_ID,rpc ) )
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}

	bool ForwardSellPointSysOP( unsigned int type,const void* pParams,size_t param_len,object_interface obj_if )
	{
		try
		{
			Marshal::OctetsStream os( Octets(pParams,param_len) );
			switch ( type )
			{
				CASE_PROTO_HANDLE(SellPoint);
				CASE_PROTO_HANDLE(SellCancel);
				CASE_PROTO_HANDLE(GetSellList);
				CASE_PROTO_HANDLE(BuyPoint);
				default:
					return false;
			}
			return true;
		}
		catch ( Marshal::Exception e )
		{
			return false;
		}
	}
}
