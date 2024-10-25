
TOP_SRCDIR = ..

SINGLE_THREAD = true
DEBUG_VERSION = false

include ../mk/gcc.defs.mk

DEFINES += -DUSE_LOGCLIENT
CLEAN += glog.o

OBJS = logclientclient.o logclienttcpclient.o state.o stubs.o logclient.o log.o

all : logclient

logclient : $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) 

include ../mk/gcc.rules.mk

