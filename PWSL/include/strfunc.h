#ifndef __STR_FUNC_H_
#define __STR_FUNC_H_

#ifdef __cplusplus
extern "C" {
#endif

char * trim		(char * str);
char * trimleft		(char * str);
char * trimright	(char * str);
char * lowerstring	(char * str);
char * upperstring	(char * str);
char * lowernstring	(char * str, size_t size);
char * uppernstring	(char * str, size_t size);

#ifdef __cplusplus
}
#endif
#endif
