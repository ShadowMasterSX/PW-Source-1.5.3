#include "types.h"
#include "obj_interface.h"
#include "kingelectionsyslib.h"
#include "libcommon.h"

#include "gproviderclient.hpp"
#include "kegetstatus.hpp"
#include "kecandidateapply.hpp"
#include "kevoting.hpp"

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
#define KE_CANDIDATE_APPLY_ITEM_ID	21652
#define KE_VOTING_ITEM_ID			38207
#define KE_VOTING_ITEM_ID2			38208
#define KE_LEVEL_REQUIRED 			100
#define KE_SEC_LEVEL_REQUIRED 		20

namespace GNET
{

bool Handle_KEGetStatus(KEGetStatus & proto, object_interface& obj_if)
{
	if(proto.roleid != obj_if.GetSelfID().id) return false;
	return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto);
}

bool Handle_KECandidateApply(KECandidateApply & proto, object_interface& obj_if)
{
	if(proto.roleid != obj_if.GetSelfID().id) return false;
	if(obj_if.GetHistoricalMaxLevel() < KE_LEVEL_REQUIRED || obj_if.GetBasicProp().sec_level < KE_SEC_LEVEL_REQUIRED) return false;
	if(proto.item_id != KE_CANDIDATE_APPLY_ITEM_ID || proto.item_num <= 0 || !obj_if.CheckItem(proto.item_id, proto.item_num)) return false;
	
	GMailSyncData syncdata;
	if(!GetSyncData(syncdata, obj_if)) return false;
	proto.syncdata = Marshal::OctetsStream() << syncdata;

	if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		obj_if.TradeUnLockPlayer();		
	}
	return false;
}

bool Handle_KEVoting(KEVoting & proto, object_interface& obj_if)
{
	if(proto.roleid != obj_if.GetSelfID().id || proto.candidate_roleid <= 0) return false;
	if(obj_if.GetHistoricalMaxLevel() < KE_LEVEL_REQUIRED || obj_if.GetBasicProp().sec_level < KE_SEC_LEVEL_REQUIRED) return false;
	if(proto.item_id != KE_VOTING_ITEM_ID && proto.item_id != KE_VOTING_ITEM_ID2 
			|| proto.item_num != 1 || !obj_if.CheckItem(proto.item_pos, proto.item_id, proto.item_num)) return false;
	
	GMailSyncData syncdata;
	if(!GetSyncData(syncdata, obj_if)) return false;
	proto.syncdata = Marshal::OctetsStream() << syncdata;

	if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		obj_if.TradeUnLockPlayer();		
	}
	return false;
}

bool ForwardKingElectionSysOP(unsigned int type, const void * pParams, size_t param_len,object_interface obj_if)
{
	try {
		Marshal::OctetsStream os( Octets(pParams,param_len) );
		switch ( type )
		{
			CASE_PROTO_HANDLE(KEGetStatus)
			CASE_PROTO_HANDLE(KECandidateApply)
			CASE_PROTO_HANDLE(KEVoting)
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
