
#include "common/platform.h"

#if defined(__x86_64__)
	#include "marshal_x86_64.h"
#elif defined(__i386__)
	#include "marshal_i386.h"
#endif
