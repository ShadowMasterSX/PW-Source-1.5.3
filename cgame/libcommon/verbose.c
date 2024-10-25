#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "verbose.h"

int glb_verbose = 0;

static enum VERBOSE_MODE verbose_mode = VERBOSE_NORMAL;
static enum VERBOSE_LEVEL_MODE level_mode= VERBOSE_LEVEL_LOWER;
static FILE *verbose_file = NULL;

inline static int check_verbose(int lvl)
{
	switch(level_mode)
	{
		case VERBOSE_LEVEL_HIGHER:
			return lvl > glb_verbose;
		case VERBOSE_LEVEL_LOWER:
			return lvl < glb_verbose;
		case VERBOSE_LEVEL_EQUAL:
			return lvl == glb_verbose;
		case VERBOSE_LEVEL_ALL:
			return 1;
	}
	return 0;
}

int verbose(int lvl, const char * data)
{
	switch(verbose_mode)
	{
		case VERBOSE_NORMAL:
			if(check_verbose(lvl))
			{
				printf("%s",data);
				fflush(stdout);
			}
		break;
		case VERBOSE_FILE:
			if(verbose_file && check_verbose(lvl))
			{
				fprintf(verbose_file, "%s",data);
				fflush(verbose_file);
			}
			
		break;
		case VERBOSE_NULL:
		default:
		break;
	}
	return 0;
}

int verbosef(int lvl,const char * fmt, ...)
{
	char buf[1024];
	va_list ap;
	if(verbose_mode == VERBOSE_NULL || !check_verbose(lvl)) return 0;
	va_start(ap, fmt);
	(void) vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);

	switch(verbose_mode)
	{
		case VERBOSE_NORMAL:
		 	printf("%s",buf);
			fflush(stdout);
		break;
		case VERBOSE_FILE:
			if(verbose_file)
			{
				fprintf(verbose_file, "%s",buf);
				fflush(verbose_file);
			}
		break;
		default:
		break;
	}
	return 0;
}

int	set_verbose_mode(enum VERBOSE_MODE mode, char *data)
{
	if(verbose_file)
	{
		fclose(verbose_file);
		verbose_file = NULL;
	}
	switch(mode)
	{
		case VERBOSE_NORMAL:
			verbose_mode = VERBOSE_NORMAL;
		break;
		case VERBOSE_FILE:
			verbose_file = fopen(data,"ab+");
			if(verbose_file == NULL)
			{
				verbose_mode = VERBOSE_NULL;
				return -2;
			}
			verbose_mode = VERBOSE_FILE;
			break;
		break;
		case VERBOSE_NULL:
			verbose_mode = VERBOSE_NULL;
		break;
		default:
		return -1;
	}
	return 0;
}

int	set_verbose_level(enum VERBOSE_LEVEL_MODE mode ,int level)
{
	level_mode = mode;
	glb_verbose = level;
	return 0;
}

