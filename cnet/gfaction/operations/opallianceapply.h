#ifndef __GNET_OPALLIANCEAPPLY_H
#define __GNET_OPALLIANCEAPPLY_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpAllianceapply : public Operation
{
#include "opallianceapply.inl"
private:
	bool ConditionCheck() { return __SYNCDATA__.player_money >= ALLIANCE_APPLY_FEE
									&& params.dst_fid > 0 && params.dst_fid != (int)player.faction_id; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			DEBUG_PRINT("opallianceapply: checkall failed. retcode=%d. rid=%d dst_fid=%d\n",retcode,oper_roleid,params.dst_fid);
			gfs_send_allianceapply_re( oper_roleid,retcode );
			return retcode;
		}
		//TODO: do operation here
		retcode = Factiondb::GetInstance()->AllianceApply(player.faction_id, params.dst_fid, WRAPPER_WREF);
		if(retcode != ERR_SUCCESS)
		{
			DEBUG_PRINT("opallianceapply: factiondb failed. retcode=%d. rid=%d dst_fid=%d\n",retcode,oper_roleid,params.dst_fid);
			gfs_send_allianceapply_re( oper_roleid,retcode );
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		RpcRetcode * res = (RpcRetcode *)pResult;
		int retcode = res->retcode;
		DEBUG_PRINT("opallianceapply: roleid=%d, retcode=%d, fid=%d, dst_fid=%d\n",oper_roleid,retcode,player.faction_id,params.dst_fid);
		if ( retcode==ERR_SUCCESS )
		{
			__SYNCDATA__.player_money -= ALLIANCE_APPLY_FEE;
			__SYNCDATA__.player_sp=0;
			if (__SYNCDATA__.player_money<0) __SYNCDATA__.player_money=0;
			gfs_broadcast_allianceapply_re( player.faction_id, params.dst_fid );
			gfs_broadcast_recvrelationapply( params.dst_fid, ALLIANCE_FROM_OTHER, player.faction_id );
		}
		else
		{
			__SYNCDATA__.player_sp=0;
			gfs_send_allianceapply_re( oper_roleid,retcode );
		}
	}
};

};//end of namespace
#endif
