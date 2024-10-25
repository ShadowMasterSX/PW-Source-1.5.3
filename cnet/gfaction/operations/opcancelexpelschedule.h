#ifndef __GNET_OPCANCELEXPELSCHEDULE_H
#define __GNET_OPCANCELEXPELSCHEDULE_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpCancelexpelschedule : public Operation
{
#include "opcancelexpelschedule.inl"
private:
	bool ConditionCheck() { return Factiondb::GetInstance()->AlreadyDelayExpel(params.memberid)
	    	&& Factiondb::GetInstance()->IsSpecialMember(player.faction_id,params.memberid,_R_MEMBER); }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_factiondelayexpelannounce(oper_roleid, retcode, FC_DELAYEXPEL_CANCEL, 
								oper_roleid, params.memberid, 0);
		    	return retcode;
		}
		//TODO: do operation here
		Factiondb::GetInstance()->CancelDelayExpel(player.faction_id,params.memberid, WRAPPER_WREF);
		return retcode;
	}
	void SetResult(void* pResult)
	{
		int retcode=( (DefFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opcancelexpelschedule:: roleid=%d,dstrole %d,fid %d,retcode=%d\n",params.memberid,player.faction_id,retcode);
		if ( retcode == ERR_FC_DELAYEXPEL ) // ERR_FC_DELAYEXPEL mean success
		{
			gfs_broadcast_factiondelayexpelannounce(player.faction_id,FC_DELAYEXPEL_CANCEL,oper_roleid,params.memberid,0);
		}
		else
		{
			gfs_send_factiondelayexpelannounce(oper_roleid, retcode, FC_DELAYEXPEL_CANCEL, 
								oper_roleid, params.memberid, 0);
		}

	}
};

};//end of namespace
#endif
