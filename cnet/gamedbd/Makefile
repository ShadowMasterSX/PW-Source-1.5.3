
TOP_SRCDIR = ..

SINGLE_THREAD = false
DEBUG_VERSION = true

include ../mk/gcc.defs.mk

INCLUDES += -I$(TOP_SRCDIR) -I$(TOP_SRCDIR)/storage -I/usr/include/db4  -I$(TOP_SRCDIR)/include
#LDFLAGS  += -static 
DEFINES  += -DUSE_DB -DUSE_WDB -DUSE_LOGCLIENT -DMPPC_4WAY -DUSE_TRANSACTION -D_FILE_OFFSET_BITS=64
DEFINES += -D__USER__=\"$(USER)\"
#CFLAGS += -g -ggdb 
LDFLAGS += -L/usr/local/ssl/lib -L/usr/lib

#OBJS = $(LOGOBJS) ../storage/storage.o ../storage/storagetool.o gamedbserver.o state.o stubs.o gamedbd.o gamedbmanager.o accessdb.o mergedb.o mergedb2.o selldataimage.o tranlog.o transman.o domaincmd.o utilfunction.o forbidpolicy.o abstractplayers.o tinyxml2.o
#gcc 9.3 reversed
OBJS = tinyxml2.o abstractplayers.o forbidpolicy.o utilfunction.o domaincmd.o transman.o tranlog.o selldataimage.o mergedb2.o mergedb.o accessdb.o gamedbmanager.o gamedbd.o stubs.o state.o gamedbserver.o ../storage/storagetool.o ../storage/storage.o $(LOGOBJS)
CLEAN += *.o

all : gamedbd

gamedbd : $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ) $(LOGSTUB) $(LOGOBJ)
#	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(SHAREOBJ) $(SHARE_SOBJ)  $(LOGSTUB) $(LOGOBJ) -lssl -lcrypto $(LIBICONV)
	$(LD) $(LDFLAGS) -o $@ $(LOGOBJ) $(LOGSTUB) $(SHAREOBJ) $(OBJS) $(SHARE_SOBJ) -lssl -lcrypto $(LIBICONV)

#cashstat: $(SHAREOBJ) $(SHARE_SOBJ) ../storage/storage.o statistic.cpp
#	$(LD) $(LDFLAGS) -D_REENTRANT -o $@ statistic.cpp $(SHAREOBJ) $(SHARE_SOBJ) ../storage/storage.o -I$(INCLUDES) -DUSE_WDB
cashstat: $(SHAREOBJ) $(SHARE_SOBJ) ../storage/storage.o statistic.o $(LOGOBJ) $(LOGSTUB)
	$(LD) $(LDFLAGS) -static -o $@ statistic.o $(SHAREOBJ) $(LOGOBJ) $(LOGSTUB) ../storage/storage.o $(SHARE_SOBJ)

include ../mk/gcc.rules.mk

