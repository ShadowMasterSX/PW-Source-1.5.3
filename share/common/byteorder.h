
#include "common/platform.h"

#if defined(__x86_64__)
	#include "common/byteorder_x86_64.h"
#elif defined(__i386__)
	#include "common/byteorder_i386.h"
#endif
