#ifndef __GNET_OPEXPELMEMBER_H
#define __GNET_OPEXPELMEMBER_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"
#include "gfactionserver.hpp"

namespace GNET
{

class OpExpelmember : public Operation
{
#include "opexpelmember.inl"
private:
	bool ConditionCheck() { return Factiondb::GetInstance()->IsSpecialMember(player.faction_id,params.memberid,_R_MEMBER); }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_factionexpelmember_re(oper_roleid,retcode,params.memberid);
			return retcode;
		}
		
		if ( Factiondb::GetInstance()->AlreadyDelayExpel(params.memberid) )
		{
			retcode = ERR_FC_ALREADY_DELAYEXPEL;
			gfs_send_factionexpelmember_re(oper_roleid,retcode,params.memberid);
			return retcode;
		}

		if(Factiondb::GetInstance()->NeedDelayExpel(player.faction_id,params.memberid))
		{
			Factiondb::GetInstance()->DelayExpelMember( player.faction_id,params.memberid, WRAPPER_WREF ); 
		}
		else
		{
			Factiondb::GetInstance()->DismissMember( player.faction_id,params.memberid, WRAPPER_WREF );
		}

		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (DefFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opexpel:: roleid=%d,dstrole %d,fid %d,retcode=%d\n",oper_roleid,params.memberid,player.faction_id,retcode);
		if ( retcode == ERR_SUCCESS )
		{
			gfs_broadcast_factionexpelmember_re(player.faction_id,oper_roleid,params.memberid);
			GFactionServer::GetInstance()->UpdatePlayer(params.memberid,_FACTION_ID_INVALID,_R_UNMEMBER);
		}
		else if ( retcode == ERR_FC_DELAYEXPEL)
		{
		    	gfs_broadcast_factiondelayexpelannounce(player.faction_id,FC_DELAYEXPEL_EXECUTE,oper_roleid,
				params.memberid,Factiondb::GetInstance()->GetDelayExpelTime(params.memberid));
		}
		else
		{
			gfs_send_factionexpelmember_re(oper_roleid,retcode,params.memberid);
		}
	}
};

};//end of namespace
#endif
