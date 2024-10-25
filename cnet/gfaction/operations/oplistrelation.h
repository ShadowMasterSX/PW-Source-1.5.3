#ifndef __GNET_OPLISTRELATION_H
#define __GNET_OPLISTRELATION_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpListrelation : public Operation
{
#include "oplistrelation.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
			return retcode;
		//TODO: do operation here
		Factiondb::GetInstance()->ListRelation(player.faction_id,player.roleid);
		return retcode=ERR_FORCE_EXIT;
	}
	void SetResult(void* pResult)
	{
	}
};

};//end of namespace
#endif
