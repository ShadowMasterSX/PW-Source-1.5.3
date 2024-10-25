
TOP_SRCDIR = ..

SINGLE_THREAD = false
DEBUG_VERSION = false

include ../mk/gcc.defs.mk

DEFINES  += -D__USE_SPEC_GAMEDATASEND__ -DUSE_LOGCLIENT 

INCLUDES += -I./header -I./header/include -I./header/include/common
OUTEROBJS = $(CO_DIR)/matcher.o 
OBJS = gproviderclient.o state.o gamed.o stubs.o gsp_if.o factionlib.o mailsyslib.o auctionsyslib.o gmattr.o sellpointlib.o webtradesyslib.o sysauctionlib.o kingelectionsyslib.o pshopsyslib.o
CLEAN += *.a

LIBOBJS =  gsp_if.o factionlib.o mailsyslib.o auctionsyslib.o stubs.o state.o gproviderclient.o gmattr.o sellpointlib.o stocklib.o webtradesyslib.o sysauctionlib.o kingelectionsyslib.o pshopsyslib.o

all : lib

gamed : $(SHAREOBJ) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(SHAREOBJ) $(OBJS) $(SHARE_SOBJ)

lib :  $(LIBOBJS) $(SHAREOBJ) $(OUTEROBJS)   
	rm -f libgsPro2.a
	$(AR) crs libgsPro2.a $(LIBOBJS) $(OUTEROBJS)  

include ../mk/gcc.rules.mk
