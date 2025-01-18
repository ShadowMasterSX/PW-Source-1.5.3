.PHONY: all clean

HVERSION := $(shell uname -a)

ifneq (,$(findstring x86_64,$(HVERSION)))
	SRCDIR := x86_64
else
	SRCDIR := i386
endif

# Force 32-bit compilation regardless of OS
SRCDIR := i386

all:
	@(cd $(SRCDIR) && $(MAKE))
	cp -p $(SRCDIR)/libperf.a .

clean:
	@(cd $(SRCDIR) && $(MAKE) clean)
	rm -f libperf.a perf.lib
