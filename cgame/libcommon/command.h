/*
	command.h           Command parse modules
        auther:		    Cui Ming
        date:		    2002-1-21
	chagelog:	    
			    2002-4-3: 将回调函数重新改回原来的格式
*/

#ifndef __COMMAND_PARSE_H__
#define __COMMAND_PARSE_H__

#include <string.h>
#include "vector.h"
#include "hashtab.h"

typedef int cmd_cb(void * arg, void *arg2,int argc, char **argv, const char * argline);

namespace abase{
struct cmdnode
{
	const char *name;
	cmd_cb 	*cb;
	void    *cb_arg;
};

struct hashstr
{
typedef const char * LPCSTR;
	const char * _reference;
        hashstr(){}
        hashstr(const char * _ref) {_reference = _ref;}
        hashstr(char * _ref) {_reference = _ref;}
	inline operator LPCSTR() const { return _reference;}
        inline bool operator ==(const hashstr &rhs) const {return (!strcmp(*this,rhs));}
};

class command
{
private:
	enum { MAX_ARG_NUMBER = 32};
	hashtab<cmdnode,hashstr,_hash_function> _c_table;
	cmdnode _default_cb;
	int 	_errno;
public:
	enum { NO_ERROR = 0,ERR_NOMEMORY,ERR_EMPTY_LINE,ERR_CMD_NOT_FOUND,ERR_CMD_NO_HANDLER};
	command(cmdnode * __ctab);
	int	error()	{ return _errno;}
public:
	int parse(const char * cmdline,void * arg,char * (*filter)(char *)=NULL,const char * delim = NULL);
};

}
#endif
