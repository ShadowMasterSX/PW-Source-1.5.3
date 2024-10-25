
TOP_SRCDIR = ..

SINGLE_THREAD = false 
DEBUG_VERSION = false

include ../mk/gcc.defs.mk

OBJS = gauthserver.o state.o stubs.o gauthd.o
DEFINES += -DUSE_EPOLL -ggdb

all : gauthd

gauthd : $(SHAREOBJ) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(SHAREOBJ) $(OBJS) $(SHARE_SOBJ)

include ../mk/gcc.rules.mk

