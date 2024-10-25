
#ifndef _SHARE_PLATFORM_H_
#define _SHARE_PLATFORM_H_

#ifdef WIN32

#ifdef _WIN64
#error "__i386__ only"
#endif

#include <windows.h>

#ifndef __i386__
#define __i386__ 
#endif

#ifndef _REENTRANT_
#define _REENTRANT_
#endif

#ifndef int64_t
typedef __int64 int64_t;
#endif

#else

#include <stdint.h>

#endif

#endif // _SHARE_PLATFORM_H_
