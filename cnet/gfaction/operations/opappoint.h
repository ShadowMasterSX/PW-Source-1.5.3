#ifndef __GNET_OPAPPOINT_H
#define __GNET_OPAPPOINT_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"
#include "dbfactionpromote.hrp"

#define VALID_OCCUP( occup ) ( (occup)>=_R_MASTER && (occup)<=_R_MEMBER )
namespace GNET
{

class OpAppoint : public Operation
{
#include "opappoint.inl"
private:
	bool ConditionCheck() { return !Factiondb::GetInstance()->AlreadyDelayExpel(params.memberid) 
	    && params.newoccup>player.froleid && VALID_OCCUP(params.newoccup)
	    && Factiondb::GetInstance()->IsSpecialMember(player.faction_id,params.memberid); }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		
		if (Factiondb::GetInstance()->GetPvpMask(player.faction_id))
			retcode = ERR_FC_PVP_OPERATE_RESTRICTED;
		
		if ( retcode != ERR_SUCCESS || (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_appoint_re( oper_roleid,params.memberid,retcode,params.newoccup );
			return retcode;
		}
		retcode = Factiondb::GetInstance()->UpdateRole(player.faction_id,oper_roleid, params.memberid, 
				player.froleid,params.newoccup, WRAPPER_WREF);
		if(retcode!=ERR_SUCCESS)
		{
			gfs_send_appoint_re( oper_roleid,params.memberid,retcode,params.newoccup );
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (DBFactionPromoteRes*) pResult )->retcode;
		DEBUG_PRINT("opappoint: roleid=%d, retcode=%d(dstroleid=%d,new occup=%d)\n",oper_roleid,retcode,
				params.memberid,params.newoccup );
		if ( retcode == ERR_SUCCESS )
		{
			gfs_broadcast_appoint_re(player.faction_id,oper_roleid,params.memberid,params.newoccup);
			GFactionServer::GetInstance()->UpdatePlayer(params.memberid,player.faction_id,params.newoccup );
		}
		else
		{
			gfs_send_appoint_re( oper_roleid,params.memberid,retcode,params.newoccup );
		}
	}
};

};//end of namespace
#endif
