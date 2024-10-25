#ifndef __GNET_OPMASTERRESIGN_H
#define __GNET_OPMASTERRESIGN_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpMasterresign : public Operation
{
#include "opmasterresign.inl"
private:
	bool ConditionCheck() { return Factiondb::GetInstance()->IsSpecialMember(player.faction_id, params.newmaster, _R_VICEMASTER) 
								&&  Factiondb::GetInstance()->IsSpecialMember(player.faction_id, oper_roleid, _R_MASTER) 
								&& !Factiondb::GetInstance()->AlreadyDelayExpel(params.newmaster)
								&& !Factiondb::GetInstance()->AlreadyDelayExpel(oper_roleid); }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_masterresign_re(oper_roleid,retcode,params.newmaster);
			return retcode;
		}
		//TODO: do operation here
		return Factiondb::GetInstance()->UpdateRole(player.faction_id,oper_roleid, params.newmaster, 
				_R_MASTER, _R_MASTER, WRAPPER_WREF);
	}
	void SetResult(void* pResult)
	{
		int retcode=( (UserFactionRes*) pResult )->retcode;
		DEBUG_PRINT("opmaster_resign: roleid=%d, retcode=%d, new master=%d\n",oper_roleid,retcode,params.newmaster);
		if ( retcode == ERR_SUCCESS )
		{
			gfs_broadcast_masterresign_re(player.faction_id,params.newmaster);
			GFactionServer::GetInstance()->UpdatePlayer(oper_roleid,player.faction_id,_R_MEMBER);
			GFactionServer::GetInstance()->UpdatePlayer(params.newmaster,player.faction_id,_R_MASTER);
		}
		else
		{
			gfs_send_masterresign_re(oper_roleid,retcode,params.newmaster);
		}
	}
};

};//end of namespace
#endif
