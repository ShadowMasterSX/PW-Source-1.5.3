#ifndef __VCLIENT_MSG_H__
#define __VCLIENT_MSG_H__

#include <stdlib.h>
#include <string.h>

struct MSG
{
	int message;
	int target;
	int param;
	size_t content_length;
	char content[];
};

inline MSG * BuildMessage(int message, int target, int param, size_t content_length = 0, const void * content = NULL)
{
	MSG * msg = (MSG *)calloc(1, sizeof(MSG)+content_length);	
	msg->message = message;
	msg->target = target;
	msg->param = param;
	msg->content_length = content_length;
	if(msg->content_length)
	{
		memcpy(msg->content, content, msg->content_length);	
	}
	return msg;
}

inline void FreeMessage(MSG * msg)
{
	free(msg);
}

enum
{
	MSG_TEST	= 0,
	MSG_SWITCH_MODE,
	MSG_FOLLOW_TARGET,
	MSG_DEBUG_C2SCMD,
	MSG_CHANGE_FORCE_ATTACK,


};


#endif
