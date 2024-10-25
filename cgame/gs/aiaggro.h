#ifndef __ONLINEGAME_GS_AI_AGGRO_H__
#define __ONLINEGAME_GS_AI_AGGRO_H__

#include "ainpc.h"
#include <ASSERT.h> 

class aggro_minor_policy : public aggro_policy  
{
protected:
public:
	DECLARE_SUBSTANCE(aggro_minor_policy);
public:

	virtual void AggroTransfer(const MSG & msg)
	{
		//do nothing
		ASSERT(msg.content_length == sizeof(XID));
		XID & id = *(XID*)msg.content;
		int rage = msg.param;
		if(id.type == -1 || id.id == -1)
		{
			_alist.Clear();
			_cur_time = 0;
		}
		else
		{
			_alist.Clear();
			_alist.AddRage(id,rage);
			_cur_time = _aggro_time;
		}
	}
};

#endif

