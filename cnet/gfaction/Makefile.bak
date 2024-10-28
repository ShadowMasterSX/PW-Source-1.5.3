
TOP_SRCDIR = ..

SINGLE_THREAD = true
DEBUG_VERSION = true

include ../mk/gcc.defs.mk

INCLUDES += -I./operations
LOG_DIR = ../logclient
INCLUDES += -I$(TOP_SRCDIR)/logclient -I$(TOP_SRCDIR)/log_inl -I$(TOP_SRCDIR)/include

DEFINES += -DUSE_HASH_MAP -DUSE_LOGCLIENT
DEFINES += -D__USER__=\"$(USER)\"
LDFLAGS += -DUSE_HASH_MAP -L/usr/lib

OUTEROBJS = $(CO_DIR)/matcher.o
CLEAN += $(CO_DIR)/matcher.o

#OBJS = gfactionserver.o gproviderserver.o gfactiondbclient.o uniquenameclient.o state.o stubs.o gfaction.o factiondb.o ./operations/operwrapper.o ./operations/operation.o ./operations/privilege.o gfs_io.o settings.o 
OBJS = settings.o gfs_io.o ./operations/privilege.o ./operations/operation.o ./operations/operwrapper.o factiondb.o gfaction.o stubs.o state.o uniquenameclient.o gfactiondbclient.o gproviderserver.o gfactionserver.o 

all : gfactiond

gfactiond : $(OBJS) $(SHAREOBJ) $(OUTEROBJS) $(LOGSTUB) $(LOGOBJ)
#	$(LD) $(LDFLAGS) -o $@ $(SHAREOBJ) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ) $(LOGSTUB) $(LOGOBJ) -lpcre $(LIBICONV)
	$(LD) $(LDFLAGS) -o $@ $(LOGOBJ) $(LOGSTUB) $(OUTEROBJS) $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) -lpcre $(LIBICONV)

include ../mk/gcc.rules.mk

