#ifndef __GNET_OPALLIANCEREPLY_H
#define __GNET_OPALLIANCEREPLY_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpAlliancereply : public Operation
{
#include "opalliancereply.inl"
private:
	bool ConditionCheck() { return params.agree && __SYNCDATA__.player_money >= AGREE_ALLIANCE_FEE
								|| !params.agree && __SYNCDATA__.player_money >= DISAGREE_ALLIANCE_FEE; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			DEBUG_PRINT("opalliancereply: checkall failed. retcode=%d. rid=%d dst_fid=%d agree=%d\n",retcode,oper_roleid,params.dst_fid,params.agree);
			gfs_send_alliancereply_re( oper_roleid,retcode );
			return retcode;
		}
		//TODO: do operation here
		retcode = Factiondb::GetInstance()->AllianceReply(player.faction_id, params.dst_fid, params.agree, WRAPPER_WREF);
		if(retcode != ERR_SUCCESS)
		{
			DEBUG_PRINT("opalliancereply: factiondb failed. retcode=%d. rid=%d dst_fid=%d agree=%d\n",retcode,oper_roleid,params.dst_fid,params.agree);
			gfs_send_alliancereply_re( oper_roleid,retcode );
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		RpcRetcode * res = (RpcRetcode *)pResult;
		int retcode = res->retcode;
		DEBUG_PRINT("opalliancereply: roleid=%d, retcode=%d, fid=%d, dst_fid=%d, agree=%d\n",oper_roleid,retcode,player.faction_id,params.dst_fid,params.agree);
		if ( retcode==ERR_SUCCESS )
		{
			__SYNCDATA__.player_money -= (params.agree ? AGREE_ALLIANCE_FEE : DISAGREE_ALLIANCE_FEE);
			__SYNCDATA__.player_sp=0;
			if (__SYNCDATA__.player_money<0) __SYNCDATA__.player_money=0;
			gfs_broadcast_alliancereply_re( player.faction_id, params.dst_fid, params.agree );
			gfs_broadcast_recvrelationreply( params.dst_fid, ALLIANCE_TO_OTHER, params.agree, player.faction_id );
		}
		else
		{
			__SYNCDATA__.player_sp=0;
			gfs_send_alliancereply_re( oper_roleid,retcode );
		}
	}
};

};//end of namespace
#endif
