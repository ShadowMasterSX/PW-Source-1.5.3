#ifndef __ONLINEGAME_AI_TRIGGER_MAN_H__
#define __ONLINEGAME_AI_TRIGGER_MAN_H__

#include <hashtab.h>

namespace ai_trigger
{
	class policy;
	class manager
	{
		abase::hashtab<policy *,int ,abase::_hash_function > _map;
	public:
		bool AddPolicy(policy * p);
		manager():_map(100)
		{}

	public:
		const policy * GetPolicy(int id) const
		{
			policy ** tmp = _map.nGet(id);
			if(tmp) 
				return *tmp;
			else
				return NULL;
		}
	};
};

#endif

