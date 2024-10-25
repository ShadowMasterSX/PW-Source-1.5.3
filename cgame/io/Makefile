
BSDTHREAD = -pthread -D_REENTRANT -D_THREAD_SAFE
#CFLAGS	= -Wall -DUSE_KEVENT -c $(BSDTHREAD)
CFLAGS = -Wall -c -pthread
#CFLAGS	= -Wall -c -pthread

OBJS = pollio.o 

.cpp.o:	FORCE
	$(CPP) -c  $<

lib: $(OBJS)

clean:
	rm -f $(OBJS) main

FORCE:

include ../Rules.make
