#ifndef __GNET_OPCREATE_H
#define __GNET_OPCREATE_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

#include "gfactionserver.hpp"
#include "gfs_io.h"	
#include "addfactionres"
#include "matcher.h"
namespace GNET
{

class OpCreate : public Operation
{
#include "opcreate.inl"
private:
	bool ConditionCheck() 
	{
	   	return params.level>=20 && params.money>=100000; 
	}
	//send to uniquename server
	void Send2UNS( int retcode,const FactionOPAddInfo& addinfo )
	{
		uns_send_postcreatefaction( 
			(char)retcode==ERR_SUCCESS,
			GFactionServer::GetInstance()->zoneid,
			addinfo.factionid,
			params.faction_name
		);
	}
public:
	int QueryAddInfo()
	{ 
		if ( !blDecodeContext && !DecodeContext() )
			return ERR_FC_DATAERROR;
	    if( params.faction_name.size()>18 || Matcher::GetInstance()->Match((char*)params.faction_name.begin(),params.faction_name.size())!=0)
		{
			gfs_send_factioncreate_re(oper_roleid,ERR_FC_INVALIDNAME,0/*new factionid*/,Octets());
	        return ERR_FC_INVALIDNAME;
		}
		DEBUG_PRINT("opcreate: query faction id to uniquename server. roleid=%d,zoneid=%d,fname(sz:%d)\n",
				oper_roleid,GFactionServer::GetInstance()->zoneid,params.faction_name.size() );
		uns_send_precreatefaction(
				GFactionServer::GetInstance()->zoneid,
				params.faction_name,
				WRAPPER_WREF
			);
		return ERR_SUCCESS; 
	}
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		FactionOPAddInfo addinfo=pWrapper->GetAddInfo();
		if ( addinfo.retcode!=ERR_SUCCESS )
		{
			DEBUG_PRINT("opcreate: uniquename failed(err:%d). rid=%d\n",addinfo.retcode,oper_roleid);
			retcode=addinfo.retcode;
			gfs_send_factioncreate_re(oper_roleid,retcode,0/*new factionid*/,Octets()); //only send to client
			return retcode;
		}
		if ( (retcode=CheckAll())!=ERR_SUCCESS )
		{
			DEBUG_PRINT("opcreate: checkall failed. retcode=%d. rid=%d\n",retcode,oper_roleid);
			gfs_send_factioncreate_re(oper_roleid,retcode,0/*new factionid*/,Octets());
			Send2UNS(retcode,addinfo);
			return retcode;
		}
		//TODO: do operation here
		retcode=Factiondb::GetInstance()->CreateFaction(addinfo.factionid,params.faction_name,oper_roleid,WRAPPER_WREF);
		if ( retcode!=ERR_SUCCESS )
		{
			DEBUG_PRINT("opcreate: factiondb failed. retcode=%d. rid=%d\n",retcode,oper_roleid);
			gfs_send_factioncreate_re(oper_roleid,retcode,0/*new factionid*/,Octets());
			Send2UNS(retcode,addinfo);
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		AddFactionRes* res=(AddFactionRes*) pResult;
		FactionOPAddInfo addinfo=pWrapper->GetAddInfo();

		DEBUG_PRINT("retcode of addfacton is %d,faction name is %.*s, roleid=%d\n",res->retcode,params.faction_name.size(),(char*)params.faction_name.begin(),oper_roleid);
		int retcode=res->retcode;
		if (retcode == ERR_SUCCESS)
		{
			__SYNCDATA__.player_money-=100000;		//money is absolute value
			__SYNCDATA__.player_sp=0;
			if (__SYNCDATA__.player_money<0)
				 __SYNCDATA__.player_money=0;		//money is absolute value
			GFactionServer::GetInstance()->UpdatePlayer(oper_roleid,addinfo.factionid,_R_MASTER);
		}
		else
			__SYNCDATA__.player_sp=0;
		//TODO: send client result
		gfs_send_factioncreate_re(oper_roleid,retcode,addinfo.factionid,params.faction_name);
		Send2UNS(retcode,addinfo);
	}
};

};//end of namespace
#endif
