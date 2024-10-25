#ifndef __GNET_OPACCELERATEEXPELSCHEDULE_H
#define __GNET_OPACCELERATEEXPELSCHEDULE_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpAccelerateexpelschedule : public Operation
{
#include "opaccelerateexpelschedule.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
			return retcode;
		//TODO: do operation here
		Factiondb::GetInstance()->SettleDelayExpel(params.wastetime);
		return retcode;
	}
	void SetResult(void* pResult)
	{
	}
};

};//end of namespace
#endif
