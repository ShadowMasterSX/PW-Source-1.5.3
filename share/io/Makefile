# Makefile

TOP_SRCDIR := ..

include ../mk/gcc.defs.mk

OUTEROBJS := ../common/octets.o ../common/thread.o ../common/conf.o ../common/timer.o ../common/itimer.o
OBJS := pollio.o protocol.o security.o rpc.o proxyrpc.o ../perf/i386/md5.o ../perf/i386/mppc256.o ../perf/i386/rc4.o ../perf/i386/sha1.o

OUTEROBJS_M := $(OUTEROBJS)
OBJS_M := pollio.o protocol.o security.o rpc.o proxyrpc.o

EXES := client server

.PHONY: all clean lib FORCE

all: $(EXES)

lib: FORCE
	$(AR) crs libgsio.a $(OUTEROBJS_M) $(OBJS_M)

server: $(OUTEROBJS) $(OBJS) server.o
	$(CXX) $(LDFLAGS) $^ -o $@

client: $(OUTEROBJS) $(OBJS) client.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(EXES) $(OUTEROBJS) $(OBJS) *.o libgsio.a

FORCE:

include ../mk/gcc.rules.mk
