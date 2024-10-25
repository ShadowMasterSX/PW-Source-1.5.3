
TOP_SRCDIR = ..
SINGLE_THREAD = false
DEBUG_VERSION = true

include ../mk/gcc.defs.mk

INCLUDES += -I$(TOP_SRCDIR)/gfaction/operations -I$(TOP_SRCDIR)/include
DEFINES += -D_TESTCODE -DUSE_HASH_MAP
LDFLAGS	+= -DUSE_HASH_MAP

OBJS = glinkclient.o state.o stubs.o gclient.o factionlib.o vclient_if.o 

SUBDIR = vclient
VCLIENTOBJS = vclient/*.o
SUBOBJS = $(VCLIENTOBJS)
CLEAN += $(SUBOBJS) + gclient

all : gclient

gclient : $(SHAREOBJ) $(OBJS) $(SUBDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) $(SUBOBJS)

$(SUBDIR): FORCE
	cd $@; make

FORCE:

include ../mk/gcc.rules.mk

