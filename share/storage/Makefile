
TOP_SRCDIR = ..

SINGLE_THREAD = false
DEBUG_VERSION = true

include ../mk/gcc.defs.mk
INCLUDES += -I/usr/include/db4
LDFLAGS  += -ldb_cxx-4.2
DEFINES += -DUSE_BDB -DUSE_WDB -DMPPC_4WAY -DUSE_TRANSACTION -D_FILE_OFFSET_BITS=64

OBJS = storage.o
CLEAN += *.o

all : tools bdbwdb_convert

tools : $(OBJS) $(SHAREOBJ) tools.o
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) tools.o

bdbwdb_convert : $(OUTEROBJS) $(OBJS) convertdb.o
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) convertdb.o

lib: $(LIBOBJS)
	rm libdb.a -f
	$(AR) crs libdb.a $(OBJS)
include ../mk/gcc.rules.mk

