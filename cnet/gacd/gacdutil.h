#ifndef __GNET_GACDUTIL
#define __GNET_GACDUTIL

#include <stdio.h>
#include <stdarg.h>

#define USERID2ACCOUNTID(iUserID) (iUserID&0xfffffff0)
inline void gacdprint(const char* msg, const char *fmt, va_list args)
{
	printf("[%s]:\t", msg);
	printf(fmt, args);
}

#define MAKE_PRINT(XXX) \
inline void gacdprint##XXX(const char* fmt, ... ) \
{ \
	printf("[%s]\t", #XXX); \
	va_list args;  \
	va_start(args, fmt); \
	vprintf(fmt, args); \
	va_end(args); \
}
 
MAKE_PRINT(DEBUG)
MAKE_PRINT(INIT)
MAKE_PRINT(CODE)
MAKE_PRINT(CLIENTINFO)
MAKE_PRINT(QUERY)
MAKE_PRINT(PUNISH)


#define GACD_STD_DEBUG_PRINT gacdprintDEBUG
#define DEBUG_PRINT_ERROR gacdprintDEBUG

#ifdef GACD_DEBUG_PRINT_NONE
	#undef GACD_DEBUG_PRINT_INIT
	#undef GACD_DEBUG_PRINT_CODE
	#undef GACD_DEBUG_PRINT_QUERY
	#undef GACD_DEBUG_PRINT_CLIENTINFO
	#undef GACD_DEBUG_PRINT_PUNISH
	#undef GACD_DEBUG_PRINT_DEBUG
#else
#ifdef GACD_DEBUG_PRINT_ALL
	#define GACD_DEBUG_PRINT_INIT
	#define GACD_DEBUG_PRINT_CODE
	#define GACD_DEBUG_PRINT_QUERY
	#define GACD_DEBUG_PRINT_CLIENTINFO
	#define GACD_DEBUG_PRINT_PUNISH
	#define GACD_DEBUG_PRINT_DEBUG
#endif
#endif

#ifdef GACD_DEBUG_PRINT_DEBUG
	#define DEBUG_PRINT_DEBUG gacdprintDEBUG
#else
	#define DEBUG_PRINT_DEBUG(...)
#endif

#ifdef GACD_DEBUG_PRINT_INIT
	#define DEBUG_PRINT_INIT gacdprintINIT
#else
	#define DEBUG_PRINT_INIT(...)
#endif

#ifdef GACD_DEBUG_PRINT_CODE
	#define DEBUG_PRINT_CODE gacdprintCODE
#else
	#define DEBUG_PRINT_CODE(...)
#endif

#ifdef GACD_DEBUG_PRINT_QUERY
	#define DEBUG_PRINT_QUERY gacdprintQUERY
#else
	#define DEBUG_PRINT_QUERY(...)
#endif

#ifdef GACD_DEBUG_PRINT_PUNISH
	#define DEBUG_PRINT_PUNISH gacdprintPUNISH
#else
	#define DEBUG_PRINT_PUNISH(...)
#endif

#ifdef GACD_DEBUG_PRINT_CLIENTINFO
	#define DEBUG_PRINT_CLIENTINFO gacdprintCLIENTINFO
#else
	#define DEBUG_PRINT_CLIENTINFO(...)
#endif

#endif
