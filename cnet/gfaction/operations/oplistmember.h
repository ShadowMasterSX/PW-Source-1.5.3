#ifndef __GNET_OPLISTMEMBER_H
#define __GNET_OPLISTMEMBER_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{
#define ERR_FORCE_EXIT 1
class OpListmember : public Operation
{
#include "oplistmember.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
		{
			gfs_send_listmember_re(oper_roleid,retcode);
			return retcode;
		}
		//TODO: do operation here
		Factiondb::GetInstance()->ListMember(player.faction_id,player.roleid,params.handle);
		return retcode=ERR_FORCE_EXIT;
	}
	void SetResult(void* pResult)
	{
	}
};

};//end of namespace
#endif
