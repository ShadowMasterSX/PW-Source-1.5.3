#include "types.h"
#include "obj_interface.h"
#include "factionlib.h"

#include "factionopsyncinfo"
#include "../gfaction/operations/factiondata.hxx"
#include "../gfaction/operations/ids.hxx"
#include "../gdbclient/db_if.h"

#include "gproviderclient.hpp"
#include "factionoprequest.hpp"
#include "playerfactioninfo.hpp"
#include "factionbeginsync_re.hpp"
#include "battlechallenge.hpp"
#include "battlechallengemap.hpp"
#include "battleserverregister.hpp"
#include "battleend.hrp"
#include "battleenter.hpp"
#include "battlestart_re.hpp"
#include "libcommon.h"
#include "sendbattlechallenge.hpp"
#include "factionserverregister.hpp"
#include "notifyfactionfortressstate.hpp"
#include "createfactionfortress.hpp"
#include "factionfortressenter.hpp"
#include "factionfortresslist.hpp"
#include "factionfortresschallenge.hpp"
#include "factionfortressbattlelist.hpp"
#include "factionfortressget.hpp"
#include "putfactionfortress.hrp"
#include "getfactionfortress.hrp"
#include "factionrenamegsverify_re.hpp"
bool get_faction_fortress_create_cost(int* cost, size_t& size);
bool get_faction_fortress_initial_value(int* technology, size_t& tsize, int* material, size_t& msize, int* building, size_t& bsize);

#define _FACTIONSERVER_ID 101
#define GDELIVERY_SERVER_ID  0
#define CASE_PROTO_HANDLE(_proto_name_)\
	case _proto_name_::PROTOCOL_TYPE:\
	{\
		_proto_name_ proto;\
		proto.unmarshal( os );\
		if ( proto.GetType()!=_proto_name_::PROTOCOL_TYPE || !proto.SizePolicy(os.size()) )\
			return false; \
		return Handle_##_proto_name_( proto,obj_if );\
	}

namespace GNET
{
	void AccelerateExpelschedule(int wastetime)
	{
	    GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID, 
		    FactionOPRequest(_O_FACTION_ACCELERATEEXPELSCHEDULE,0,accelerateexpelschedule_param_st(wastetime).marshal()) );
	}
    
   	bool ForwardFactionOP( int optype,int roleid,const void* pParams,size_t param_len,object_interface obj_if )	
	{
		if ( pParams==NULL || param_len<0 ) return false;
		
		Octets clientParams(pParams,param_len);
		//size check
		if ( param_len+12>128/*max FactionOPRequest size*/ )	
			return false;
		try
		{
		switch (optype)
		{
		case _O_FACTION_TESTSYNC:
			clientParams=Marshal::OctetsStream()<<(int)100;
			return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,
					FactionOPRequest(optype,roleid,clientParams) );
			break;		
		case _O_FACTION_CREATE:
			return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,
					FactionOPRequest(optype,roleid,
						 create_param_st(
							obj_if.GetBasicProp().level/*level*/,
							obj_if.GetMoney()/*money*/,
							obj_if.GetBasicProp().skill_point/*sp*/,
							create_param_ct().Create(clientParams)
							).marshal() )
					);
			break;
		case _O_FACTION_UPGRADE:
			return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,
					FactionOPRequest(optype,roleid,
						upgrade_param_st(
							obj_if.GetMoney()/*money*/,
							0/*padding*/
							).marshal() )
						);	
			break;
		case _O_FACTION_CHANGEPROCLAIM:
		case _O_FACTION_ACCEPTJOIN:
		case _O_FACTION_EXPELMEMBER:
		case _O_FACTION_APPOINT:
		case _O_FACTION_MASTERRESIGN:
		case _O_FACTION_RESIGN:
		case _O_FACTION_LEAVE:
		case _O_FACTION_BROADCAST:
		case _O_FACTION_DISMISS:
		case _O_FACTION_DEGRADE:
		case _O_FACTION_LISTMEMBER:	
		case _O_FACTION_RENAME:
		case _O_FACTION_ALLIANCEAPPLY:
		case _O_FACTION_ALLIANCEREPLY:
		case _O_FACTION_HOSTILEAPPLY:
		case _O_FACTION_HOSTILEREPLY:
		case _O_FACTION_REMOVERELATIONAPPLY:
		case _O_FACTION_REMOVERELATIONREPLY:
		case _O_FACTION_LISTRELATION:
		case _O_FACTION_CANCELEXPELSCHEDULE:	
		case _O_FACTION_FACTIONRENAME:
			return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,
					FactionOPRequest(optype,roleid,clientParams) );
			break;	
		default:
			return false;
			break;	
		}//end of switch
		}//end of try
		catch (Marshal::Exception )
		{
			return false;
		}
	}

	bool QueryPlayerFactionInfo( int roleid )
	{
		PlayerFactionInfo pfi;
		pfi.rolelist.add(roleid);
		return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,pfi);
	}

	bool QueryPlayerFactionInfo( const int* list, int list_len )
	{
		PlayerFactionInfo pfi;
		if (list==NULL || list_len<=0) return false;
		for( ; list_len-- ; pfi.rolelist.add(*list++) );
		return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,pfi);
	}

	bool SendFactionLockResponse(int retcode,int tid,int roleid,const syncdata_t& syncdata)
	{
		return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,
					FactionBeginSync_Re(retcode,tid,roleid,FactionOPSyncInfo(syncdata.money,syncdata.sp))
				);

	}

	bool SendBattleEnd(int battle_id, int result, int defender, int attacker)
	{
		BattleEndArg arg(battle_id, result, defender, attacker);
		Rpc *rpc = Rpc::Call(RPC_BATTLEEND, &arg);
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,rpc);
	}

	bool ResponseBattleStart(int battle_id, int retcode)
	{
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,BattleStart_Re(battle_id, retcode));
	}

	bool SendBattleServerRegister(int map_type, int server_id, int world_tag)
	{
		BattleServerRegister proto(map_type, server_id, world_tag);
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);
	}

	bool Handle_BattleChallenge( BattleChallenge& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		if( (int)proto.factionid!=obj_if.GetMafiaID())
			return false;
		if(proto.deposit < 1 || proto.deposit > 99) return false;
		if(!obj_if.CheckItem(PWRD_SILVER_NOTE_ID, proto.deposit)) return false;

		SendBattleChallenge request(proto.roleid, proto.id, proto.factionid, proto.deposit, proto.authentication);
		request.localsid = obj_if.GetLinkSID();

		if (!GetSyncData(request.syncdata,obj_if)) 
			return false;

		/*if(request.syncdata.inventory.money < proto.deposit)
			return false;*/
		
		if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if(GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,request))
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}	

	bool Handle_BattleChallengeMap( BattleChallengeMap& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		if( proto.factionid!=obj_if.GetMafiaID())
			return false;
		proto.localsid = obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);
	}	

	bool Handle_BattleEnter( BattleEnter& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		proto.localsid = obj_if.GetLinkSID();
		return GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,proto);
	}	

	bool ForwardBattleOP(unsigned int type,const void* pParams,size_t param_len,object_interface obj_if )
	{
		try {
			Marshal::OctetsStream os( Octets(pParams,param_len) );
			switch (type)
			{
				CASE_PROTO_HANDLE(BattleEnter)
				CASE_PROTO_HANDLE(BattleChallenge)
				CASE_PROTO_HANDLE(BattleChallengeMap)
				default:
					return false;	
			}
		}
		catch ( Marshal::Exception )
		{
			return false;
		}
	}
	

	bool SendFactionServerRegister(int server_id, int world_tag)
	{
		FactionServerRegister proto(server_id, world_tag);
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);
	}

	bool Handle_CreateFactionFortress( CreateFactionFortress& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		if(proto.factionid != obj_if.GetMafiaID() || !proto.factionid)
			return false;
		if(obj_if.GetMafiaRank() != _R_MASTER)	//只有帮主可以
			return false;

		int cost[16];
		size_t size = 16;
		if(!get_faction_fortress_create_cost(cost,size))
			return false;
		if(size%2 != 0)
			return false;
		for(size_t i=0; i<size; i+=2)
		{
			if(!obj_if.CheckItem(cost[i],cost[i+1])) 
				return false;
		}
		proto.item_cost.replace(cost,size*sizeof(int));
		
		int technology[5], material[8], building[40];
		size_t tsize = 5, msize = 8, bsize = 40;
		if(!get_faction_fortress_initial_value(technology, tsize, material, msize, building, bsize))
			return false;
		GFactionFortressInfo info;
		info.level = 1;
		info.technology.replace(technology, tsize*sizeof(int));
		info.material.replace(material, msize*sizeof(int));
		info.building.replace(building, bsize*sizeof(int));
		proto.fortress_info = Marshal::OctetsStream() << info;

		GMailSyncData syncdata;
		if( !GetSyncData(syncdata,obj_if) )
			return false;
		proto.syncdata = Marshal::OctetsStream() << syncdata;

		if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if(GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,proto))
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}	

	bool Handle_FactionFortressEnter( FactionFortressEnter& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		if(proto.factionid != obj_if.GetMafiaID() || !proto.factionid || !proto.dst_factionid)
			return false;
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);
	}

	bool Handle_FactionFortressList( FactionFortressList& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);	
	}
	
	bool Handle_FactionFortressChallenge( FactionFortressChallenge& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		if(proto.factionid != obj_if.GetMafiaID() || !proto.factionid || !proto.target_factionid)
			return false;
		if(obj_if.GetMafiaRank() != _R_MASTER && obj_if.GetMafiaRank() != _R_VICEMASTER)	//只有帮主副帮主可以
			return false;
		if(obj_if.GetMoney() < FACTION_FORTRESS_CHALLENGE_FEE)
			return false;
		
		GMailSyncData syncdata;
		if( !GetSyncData(syncdata,obj_if) )
			return false;
		proto.syncdata = Marshal::OctetsStream() << syncdata;
		
		if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto))
				return true;
			obj_if.TradeUnLockPlayer();
		}
		return false; 
	}

	bool Handle_FactionFortressBattleList( FactionFortressBattleList& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);	
	}
	
	bool Handle_FactionFortressGet( FactionFortressGet& proto,object_interface& obj_if )
	{
		if ( proto.roleid!=obj_if.GetSelfID().id ) 
			return false;
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);	
	}
	
	bool ForwardFactionFortressOP(unsigned int type,const void* pParams,size_t param_len,object_interface obj_if)
	{
		try {
			Marshal::OctetsStream os( Octets(pParams,param_len) );
			switch (type)
			{
				CASE_PROTO_HANDLE(CreateFactionFortress)
				CASE_PROTO_HANDLE(FactionFortressEnter)
				CASE_PROTO_HANDLE(FactionFortressList)
				CASE_PROTO_HANDLE(FactionFortressChallenge)
				CASE_PROTO_HANDLE(FactionFortressBattleList)
				CASE_PROTO_HANDLE(FactionFortressGet)
				default:
					return false;	
			}
		}
		catch ( Marshal::Exception )
		{
			return false;
		}
	}
	
	bool SendFactionFortressState(int factionid, int state)
	{
		NotifyFactionFortressState proto(factionid,state);
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,proto);
	}
	
	namespace
	{
		inline void push_back(Octets & os, const ivec & vec)
		{
			if(vec.size) os.insert(os.end(),vec.data,vec.size);
		}
		inline void get_buf(const Octets & os, ivec & vec)
		{
			if((vec.size = os.size()))
			{
				vec.data = os.begin();
			}
			else
			{
				vec.data = NULL;
			}
		}
	}
	void FactionFortressData2DB(GFactionFortressInfo * pData, const faction_fortress_data & data)
	{
		pData->level 		= data.level;	
		pData->exp 			= data.exp;	
		pData->exp_today 	= data.exp_today;	
		pData->exp_today_time = data.exp_today_time;	
		pData->tech_point 	= data.tech_point;	
		push_back(pData->technology, data.technology);
		push_back(pData->material, data.material);
		push_back(pData->building, data.building);
		push_back(pData->common_value, data.common_value);
		push_back(pData->actived_spawner, data.actived_spawner);
	}
	
	void DB2FactionFortressData(int factionid, const GFactionFortressInfo * pData, faction_fortress_data & data)
	{
		data.factionid 	= factionid;	
		data.level 		= pData->level;	
		data.exp 		= pData->exp;	
		data.exp_today 	= pData->exp_today;	
		data.exp_today_time = pData->exp_today_time;	
		data.tech_point = pData->tech_point;	
		get_buf(pData->technology, data.technology);
		get_buf(pData->material, data.material);
		get_buf(pData->building, data.building);
		get_buf(pData->common_value, data.common_value);
		get_buf(pData->actived_spawner, data.actived_spawner);
	}
	
	void DB2FactionFortressData(int factionid, const GFactionFortressInfo2 * pData, faction_fortress_data2 & data2)
	{
		data2.factionid  = factionid;
		data2.health     = pData->health;
		data2.offense_faction = pData->offense_faction;
		data2.offense_starttime = pData->offense_starttime;
		data2.offense_endtime = pData->offense_endtime;
	}
	
	bool get_faction_fortress(int factionid, FactionFortressResult * callback)
	{
		GetFactionFortress * rpc = (GetFactionFortress *)Rpc::Call(RPC_GETFACTIONFORTRESS,GetFactionFortressArg(factionid));
		rpc->_callback = callback;
		bool success = GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,rpc);
		if(success) PollIO::WakeUp();
		return success;
	}
	
	bool put_faction_fortress(int factionid, const faction_fortress_data * data, FactionFortressResult * callback)
	{
		PutFactionFortressArg arg;
		arg.factionid = data->factionid;
		FactionFortressData2DB(&arg.info,*data);
		PutFactionFortress * rpc = (PutFactionFortress *)Rpc::Call(RPC_PUTFACTIONFORTRESS,arg);
		rpc->_callback = callback;
		return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID,rpc);
	}
	
	bool FactionRenameRespond(int roleid,int fid,int itemid,int itemnum,int itempos,const void* name,size_t len,object_interface& obj_if)
	{
		GMailSyncData syncdata;
		if( !GetSyncData(syncdata,obj_if) )
			return false;
		Octets newname(name,len);
		
		if(obj_if.TradeLockPlayer(0, DBMASK_PUT_SYNC_TIMEOUT)==0)
		{
			if(GProviderClient::GetInstance()->DispatchProtocol(_FACTIONSERVER_ID,
						FactionRenameGsVerify_Re(roleid,fid,ERR_SUCCESS,newname,itemid,itemnum,itempos,syncdata)))	
					return true;
			obj_if.TradeUnLockPlayer();
		}
		return false;
	}
};
