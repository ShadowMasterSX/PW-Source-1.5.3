
HVERSION=$(shell uname -a)

ifneq (,$(findstring x86_64,$(HVERSION)))
	SRCDIR = x86_64
else
	SRCDIR = i386
endif
# always making 32-bit even if on 64-bit OS
SRCDIR = i386

all:
	@(cd $(SRCDIR) && make)
	cp -p $(SRCDIR)/libperf.a .
	#cp $(SRCDIR)/perf.lib .

clean:
	@(cd $(SRCDIR) && make clean)
	rm -f libperf.a perf.lib
