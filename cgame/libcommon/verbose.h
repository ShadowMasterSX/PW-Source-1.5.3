#ifndef __VERBOSE_CM_H__
#define __VERBOSE_CM_H__

#ifdef __cplusplus
extern "C" {
#endif

enum  VERBOSE_LEVEL_MODE
{
	VERBOSE_LEVEL_HIGHER,
	VERBOSE_LEVEL_LOWER,
	VERBOSE_LEVEL_EQUAL,
	VERBOSE_LEVEL_ALL
};

enum VERBOSE_MODE
{
	VERBOSE_NORMAL,
	VERBOSE_FILE,
	VERBOSE_NULL
};

int	verbose(int lvl, const char * data);
int	verbosef(int lvl, const char * format, ...);
int	set_verbose_mode(enum VERBOSE_MODE mode, char *data);
int	set_verbose_level(enum VERBOSE_LEVEL_MODE mode ,int level);

extern  int glb_verbose;
#ifdef __cplusplus
}
#endif

#endif

