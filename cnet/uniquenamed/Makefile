
TOP_SRCDIR = ..

SINGLE_THREAD = false
DEBUG_VERSION = true

include ../mk/gcc.defs.mk

INCLUDES += -I$(TOP_SRCDIR)/storage -I$(TOP_SRCDIR)/include
LDFLAGS  += -g -ggdb -L/usr/lib
CFLAGS   += -g -ggdb
DEFINES  += -DUSE_WDB -DUSE_LOGCLIENT -DMPPC_4WAY -DUSE_TRANSACTION -D_FILE_OFFSET_BITS=64 #-DCHARSET_UTF8
DEFINES += -D__USER__=\"$(USER)\"

#OBJS = ../storage/storage.o ../storage/storagetool.o uniquenameserver.o state.o stubs.o uniquenamed.o accessdb.o
OBJS = accessdb.o uniquenamed.o stubs.o state.o uniquenameserver.o ../storage/storagetool.o ../storage/storage.o

all : uniquenamed

uniquenamed : $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) $(LOGOBJ) $(LOGSTUB)
#	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) $(SHARELIB) $(LOGOBJ) $(LOGSTUB) $(LIBICONV)
	$(LD) $(LDFLAGS) -o $@ $(LOGSTUB) $(LOGOBJ) $(SHARELIB) $(SHARE_SOBJ) $(SHAREOBJ) $(OBJS)  $(LIBICONV)

include ../mk/gcc.rules.mk

