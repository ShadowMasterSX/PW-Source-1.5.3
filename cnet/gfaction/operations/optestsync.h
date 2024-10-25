#ifndef __GNET_OPTESTSYNC_H
#define __GNET_OPTESTSYNC_H
#include "operation.h"
#include "operwrapper.h"
#include "factiondata.hxx"
#include "factiondb.h"

namespace GNET
{

class OpTestsync : public Operation
{
#include "optestsync.inl"
private:
	bool ConditionCheck() { return true; }
public:
	int Execute()
	{
		int retcode=ERR_SUCCESS;
		if ( (retcode=CheckAll())!=ERR_SUCCESS)
			return retcode;
		//TODO: do operation here
		return retcode;
	}
	void SetResult(void* pResult)
	{
	}
};

};//end of namespace
#endif
