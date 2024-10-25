
TOP_SRCDIR = ..

SINGLE_THREAD = false
DEBUG_VERSION = true

include ../mk/gcc.defs.mk

#OBJS = logserviceserver.o logservicetcpserver.o state.o stubs.o logservice.o logdispatch.o 
# reverse order
OBJS =  logdispatch.o logservice.o stubs.o state.o logservicetcpserver.o logserviceserver.o

all : logservice

logservice : $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ)
#	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ)
	$(LD) $(LDFLAGS) -o $@ $(SHARE_SOBJ) $(SHAREOBJ) $(OBJS) 

include ../mk/gcc.rules.mk

