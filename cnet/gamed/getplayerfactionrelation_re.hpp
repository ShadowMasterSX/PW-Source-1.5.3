
#ifndef __GNET_GETPLAYERFACTIONRELATION_RE_HPP
#define __GNET_GETPLAYERFACTIONRELATION_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{
void ReceivePlayerFactionRelation(int roleid,unsigned int faction_id,int* alliance,size_t asize,int* hostile, size_t hsize);
class GetPlayerFactionRelation_Re : public GNET::Protocol
{
	#include "getplayerfactionrelation_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		size_t alliance_count = alliance.size();
		int * alliance_list = NULL;
		if(alliance_count)
		{
			alliance_list = (int *)malloc(sizeof(int)*alliance_count);
			for(size_t i=0; i<alliance_count; i++)
				alliance_list[i] = alliance[i];
		}
		size_t hostile_count = hostile.size();
		int * hostile_list = NULL;
		if(hostile_count)
		{
			hostile_list = (int *)malloc(sizeof(int)*hostile_count);
			for(size_t i=0; i<hostile_count; i++)
				hostile_list[i] = hostile[i];
		}
		for(size_t i=0; i<roleid_list.size(); i++)
			ReceivePlayerFactionRelation(roleid_list[i],factionid,alliance_list,alliance_count,hostile_list,hostile_count);	
		if(alliance_count)
		{
			free(alliance_list);
			alliance_list = NULL;
			alliance_count = 0;
		}
		if(hostile_count)
		{
			free(hostile_list);
			hostile_list = NULL;
			hostile_count = 0;
		}
	}
};

};

#endif
