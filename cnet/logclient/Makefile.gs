
TOP_SRCDIR = ..

SINGLE_THREAD = false
DEBUG_VERSION = false

include ../mk/gcc.defs.mk

DEFINES += -DUSE_LOGCLIENT 

OBJS = logclientclient.o logclienttcpclient.o state.o stubs.o logclient.o log.o
LIBOBJS = logclientclient.o logclienttcpclient.o state.o stubs.o glog.o log.o

all : lib

logclient : $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) 

lib: $(LIBOBJS) $(SHAREOBJ) $(SHARE_SOBJ)
	rm ./liblogCli.a -f
	$(AR) crs ./liblogCli.a $(LIBOBJS)

include ../mk/gcc.rules.mk

