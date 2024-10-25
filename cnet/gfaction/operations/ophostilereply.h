#ifndef __GNET_OPHOSTILEREPLY_H
#define __GNET_OPHOSTILEREPLY_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpHostilereply : public Operation
{
#include "ophostilereply.inl"
private:
	bool ConditionCheck() { return params.agree && __SYNCDATA__.player_money >= AGREE_HOSTILE_FEE
								|| !params.agree && __SYNCDATA__.player_money >= DISAGREE_HOSTILE_FEE; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			DEBUG_PRINT("ophostilereply: checkall failed. retcode=%d. rid=%d dst_fid=%d agree=%d\n",retcode,oper_roleid,params.dst_fid,params.agree);
			gfs_send_hostilereply_re( oper_roleid,retcode );
			return retcode;
		}
		//TODO: do operation here
		retcode = Factiondb::GetInstance()->HostileReply(player.faction_id, params.dst_fid, params.agree, WRAPPER_WREF);
		if(retcode != ERR_SUCCESS)
		{
			DEBUG_PRINT("ophostilereply: factiondb failed. retcode=%d. rid=%d dst_fid=%d agree=%d\n",retcode,oper_roleid,params.dst_fid,params.agree);
			gfs_send_hostilereply_re( oper_roleid,retcode );
		}
		return retcode;
	}
	void SetResult(void* pResult)
	{
		RpcRetcode * res = (RpcRetcode *)pResult;
		int retcode = res->retcode;
		DEBUG_PRINT("ophostilereply: roleid=%d, retcode=%d, fid=%d, dst_fid=%d, agree=%d\n",oper_roleid,retcode,player.faction_id,params.dst_fid,params.agree);
		if ( retcode==ERR_SUCCESS )
		{
			__SYNCDATA__.player_money -= (params.agree ? AGREE_HOSTILE_FEE : DISAGREE_HOSTILE_FEE);
			__SYNCDATA__.player_sp=0;
			if (__SYNCDATA__.player_money<0) __SYNCDATA__.player_money=0;
			gfs_broadcast_hostilereply_re( player.faction_id, params.dst_fid, params.agree );
			gfs_broadcast_recvrelationreply( params.dst_fid, HOSTILE_TO_OTHER, params.agree, player.faction_id );
		}
		else
		{
			__SYNCDATA__.player_sp=0;
			gfs_send_hostilereply_re( oper_roleid,retcode );
		}
	}
};

};//end of namespace
#endif
