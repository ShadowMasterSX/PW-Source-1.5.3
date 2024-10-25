#include "command.h"
#include "strtok.h"

abase::command::command(abase::cmdnode *__ctab):_c_table(10)
{
	_default_cb.name = NULL;
	_default_cb.cb  = NULL;
	_default_cb.cb_arg = NULL;
	int i;
	i = 0;
	while(1)
	{
		if(__ctab[i].name)
		{
			_c_table.put(hashstr(__ctab[i].name),__ctab[i]);
		}
		else
		if(__ctab[i].cb){
			_default_cb = __ctab[i];
		}
		else
			break;
		i ++;
	}
}

int abase::command::parse(const char * cmdline, void *arg, char * (*filter)(char *),const char * delim)
{
	int argn;
	char *argv[MAX_ARG_NUMBER];
	const char *arg_line = NULL;
	char * data;

	_errno = NO_ERROR;
	data = strdup(cmdline);
	if(!data) {
		_errno = ERR_NOMEMORY;
		return -1;
	}

	strtok token(data,delim?delim:" \t\r\n");
	for(argn = 0; argn < MAX_ARG_NUMBER;argn ++)
	{
		if((argv[argn] = token.org_token()) == NULL)
		break;
		
	}

	if(argn == 0) {
		free(data);
		_errno = ERR_EMPTY_LINE;
		return -1;
	}
	
	if(argn > 1){
		arg_line = cmdline + (argv[1] - data);
	}
	if(filter) filter(argv[0]);
	abase::pair<abase::cmdnode * , bool> node = _c_table.get(hashstr(argv[0]));
	int rst = -1;
	if(node.second)
	{	
		//match
		if(node.first->cb) 
			rst = node.first->cb(node.first->cb_arg, arg, argn, argv, arg_line);
		else
			_errno = ERR_CMD_NO_HANDLER;
	}
	else
	{
		if(_default_cb.cb){
			rst = _default_cb.cb(_default_cb.cb_arg, arg, argn, argv, arg_line);
		}
		else
			_errno = ERR_CMD_NOT_FOUND;
	}
	free(data);
	return rst;
}

