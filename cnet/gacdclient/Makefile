SINGLE_THREAD = true

TOP_SRCDIR = ..

include ../mk/gcc.defs.mk

OUTEROBJS = ../io/pollio.o ../io/protocol.o ../io/security.o ../io/rpc.o ../io/proxyrpc.o ../common/octets.o ../common/thread.o ../common/conf.o ../common/timer.o ../common/itimer.o
OBJS = commander.o ganticheaterclient.o state.o stubs.o gacdclient.o mytimer.o db.o xmlobject.o xmldb.o cheatersdb.o xmlconfigdb.o

all : gacdclient

gacdclient : $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ) -lxml2

include ../mk/gcc.rules.mk

