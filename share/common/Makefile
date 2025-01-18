# Makefile

TOP_SRCDIR := ..

include ../mk/gcc.defs.mk

OBJS := octets.o thread.o conf.o timer.o itimer.o ../io/pollio.o
EXES := marshal thread ctrl interval
CLEAN += *.o

.PHONY: all clean

all: $(EXES)

marshal: $(OBJS) marshal.o
	$(LD) $(LDFLAGS) $(OBJS) marshal.o -o $@

thread: $(OBJS) main.o
	$(LD) $(LDFLAGS) $(OBJS) main.o -o $@

ctrl: ctrl.o
	$(LD) $(LDFLAGS) $^ -o $@

interval: interval.o $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(EXES) *.o

include ../mk/gcc.rules.mk
