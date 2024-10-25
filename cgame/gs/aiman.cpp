#include "aiman.h"
#include "aitrigger.h"

namespace ai_trigger
{
bool
manager::AddPolicy(policy * p)
{
	return _map.put(p->GetID(),p);
}

}

