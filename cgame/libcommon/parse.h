#ifndef __CONF_PARSE_H__
#define __CONF_PARSE_H__

typedef struct __parse_structure{
	char * 	tag;
	int  	type;
	void * 	data;
	int 	size;
}parse_structure;
enum
{
	TYPE_CHAR = 0,
	TYPE_UCHAR,
	TYPE_SHORT,
	TYPE_USHORT,
	TYPE_INT,
	TYPE_UINT,
	TYPE_FLOAT,
	TYPE_DOUBLE,
	TYPE_STRING,
};

#ifdef __cplusplus
extern "C" {
#endif

int 	conf_parse(char * line, parse_structure * table);

#ifdef __cplusplus
}
#endif
#endif
