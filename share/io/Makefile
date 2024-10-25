#
# Makefile
#

TOP_SRCDIR = ..

include ../mk/gcc.defs.mk

#BSDTHREAD = -pthread -D_REENTRANT -D_THREAD_SAFE
#CFLAGS = -Wall -DUSE_KEVENT $(BSDTHREAD)
#CFLAGS = -Wall -DUSE_EPOLL -D_XOPEN_SOURCE=500
#CFLAGS  = -Wall -D_XOPEN_SOURCE=500 -pipe -pthread

OUTEROBJS = ../common/octets.o ../common/thread.o ../common/conf.o ../common/timer.o ../common/itimer.o
OBJS = pollio.o protocol.o security.o rpc.o proxyrpc.o ../perf/i386/md5.o ../perf/i386/mppc256.o ../perf/i386/rc4.o ../perf/i386/sha1.o #base64.o

OUTEROBJS_M = ../common/octets.o ../common/thread.o ../common/conf.o ../common/timer.o ../common/itimer.o
OBJS_M = pollio.o protocol.o security.o rpc.o proxyrpc.o #base64_m.o
#OBJS_M += ../perf/i386/md5.o ../perf/i386/mppc256.o ../perf/i386/rc4.o ../perf/i386/sha1.o

EXES = client server

all : $(EXES)

#$(OUTEROBJS_M) $(OBJS_M)

lib:  FORCE
	$(AR) crs libgsio.a $(OUTEROBJS_M) $(OBJS_M)

server: $(OUTEROBJS) $(OBJS) server.o
	g++ $(LDFLAGS) $(OUTEROBJS) $(OBJS) server.o -o $@

client: $(OUTEROBJS) $(OBJS) client.o
	g++ $(LDFLAGS) $(OUTEROBJS) $(OBJS) client.o -o $@

FORCE:

include ../mk/gcc.rules.mk

