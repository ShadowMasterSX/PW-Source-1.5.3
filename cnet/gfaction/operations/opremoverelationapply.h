#ifndef __GNET_OPREMOVERELATIONAPPLY_H
#define __GNET_OPREMOVERELATIONAPPLY_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpRemoverelationapply : public Operation
{
#include "opremoverelationapply.inl"
private:
	bool ConditionCheck() { return params.force && __SYNCDATA__.player_money >= FORCE_REMOVE_RELATION_FEE
								|| !params.force && __SYNCDATA__.player_money >= REMOVE_RELATION_FEE; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			DEBUG_PRINT("opremoverelationapply: checkall failed. retcode=%d. rid=%d dst_fid=%d force=%d\n",retcode,oper_roleid,params.dst_fid,params.force);
			gfs_send_removerelationapply_re( oper_roleid,retcode );
			return retcode;
		}
		//TODO: do operation here
		retcode = Factiondb::GetInstance()->RemoveRelationApply(player.faction_id, params.dst_fid, params.force, WRAPPER_WREF);
		if(retcode != ERR_SUCCESS)
		{
			DEBUG_PRINT("opremoverelationapply: factiondb failed. retcode=%d. rid=%d dst_fid=%d force=%d\n",retcode,oper_roleid,params.dst_fid,params.force);
			gfs_send_removerelationapply_re( oper_roleid,retcode );
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		RpcRetcode * res = (RpcRetcode *)pResult;
		int retcode = res->retcode;
		DEBUG_PRINT("opremoverelationapply: roleid=%d, retcode=%d, fid=%d, dst_fid=%d, force=%d\n",oper_roleid,retcode,player.faction_id,params.dst_fid, params.force);
		if ( retcode==ERR_SUCCESS )
		{
			__SYNCDATA__.player_money -= (params.force ? FORCE_REMOVE_RELATION_FEE : REMOVE_RELATION_FEE);
			__SYNCDATA__.player_sp=0;
			if (__SYNCDATA__.player_money<0) __SYNCDATA__.player_money=0;
			gfs_broadcast_removerelationapply_re( player.faction_id, params.dst_fid, params.force );
		}
		else
		{
			__SYNCDATA__.player_sp=0;
			gfs_send_removerelationapply_re( oper_roleid,retcode );
		}
	}
};

};//end of namespace
#endif
