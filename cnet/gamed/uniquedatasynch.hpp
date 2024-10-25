
#ifndef __GNET_UNIQUEDATASYNCH_HPP
#define __GNET_UNIQUEDATASYNCH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "guniquedataelemnode"

void OnUniqueDataLoad(int key,int type,int version,const void* p,size_t sz);
void OnUniqueDataLoadFinish();
void OnUniqueDataClose();

namespace GNET
{

class UniqueDataSynch : public GNET::Protocol
{
	#include "uniquedatasynch"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GUniqueDataElemNodeVector::iterator iter = values.begin();
		GUniqueDataElemNodeVector::iterator iend = values.end();

		for(; iter != iend; ++iter)
		{
			OnUniqueDataLoad(iter->key, iter->val.vtype, iter->val.version, iter->val.value.begin(),iter->val.value.size());
		}

		if(1 == finish) 
		{
			OnUniqueDataLoadFinish();
		}
		else if(-1 == finish) 
		{
			OnUniqueDataClose(); 
		}
	}
};

};

#endif
