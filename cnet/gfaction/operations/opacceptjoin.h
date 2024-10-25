#ifndef __GNET_OPACCEPTJOIN_H
#define __GNET_OPACCEPTJOIN_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

#include "gfs_io.h"
namespace GNET
{

class OpAcceptjoin : public Operation
{
#include "opacceptjoin.inl"
private:
	bool ConditionCheck() { 
		return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;

		if (Factiondb::GetInstance()->GetPvpMask(player.faction_id))
			retcode = ERR_FC_PVP_OPERATE_RESTRICTED;

		if ( retcode != ERR_SUCCESS || (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_factionacceptjoin_re(oper_roleid,retcode,params.applicant,oper_roleid);
			gfs_send_factionacceptjoin_re(params.applicant,retcode,params.applicant,oper_roleid);
			return retcode;
		}
		//TODO: do operation here
		/*
		if (params.blAgree == 0) //means disagreed to accept
		{
			DEBUG_PRINT("opacceptjoin:: master disagree to accept %d\n",params.applicant);
			gfs_send_factionacceptjoin_re(oper_roleid,ERR_SUCCESS,params.applicant);	
			gfs_send_factionapplyjoin_re(params.applicant,ERR_FC_JOIN_REFUSE,player.faction_id);
			return ERR_SUCCESS;
		}
		*/
		//try to accept a memeber
		retcode=Factiondb::GetInstance()->AcceptJoin(player.faction_id, params.applicant, WRAPPER_WREF);
		return retcode;
	}
	void SetResult(void* pResult)
	{
		AddMemberRes* res=(AddMemberRes*) pResult;
		int retcode=( (AddMemberRes*) pResult )->retcode;
		DEBUG_PRINT("opacceptjoin:: fid=%d,retcode=%d,inviter=%d,new member=%d\n",player.faction_id,retcode,
				oper_roleid,params.applicant);
		if (retcode==ERR_SUCCESS)
		{
			GFactionServer::GetInstance()->AddMember(player.faction_id,oper_roleid,params.applicant,
				res->level, res->cls, res->reputation, res->reincarn_times, res->gender, res->name);
		}
		else
		{
			gfs_send_factionacceptjoin_re(oper_roleid,retcode,params.applicant,oper_roleid);
			gfs_send_factionacceptjoin_re(params.applicant,retcode,params.applicant,oper_roleid);
		}
	}
};

};//end of namespace
#endif
