# Top-Level Makefile

TOP_SRCDIR := .
branchname := dbserver

# Directories
docdirs :=
extradirs :=
execdirs := /root/cnet/glinkd/ /root/cnet/gdeliveryd/ /root/cnet/gauthd/ /root/cnet/gamed/ /root/cnet/gamedbd/ /root/cnet/gdbclient/ /root/cnet/gfaction/ /root/cnet/uniquenamed/
alldirs := $(docdirs) $(extradirs) $(execdirs)

tardirs := $(alldirs)

# Rules
.PHONY: $(alldirs) clean distclean tar install uninstall

all: $(execdirs)

$(execdirs):
	$(MAKE) -C $(@D) $(@F) all

clean:
	@$(foreach dir, $(alldirs), $(MAKE) -C $(dir) clean || exit 1;)

distclean:
	@$(foreach dir, $(alldirs), $(MAKE) -C $(dir) distclean || exit 1;)

tar: distclean
	@here=$$(basename $$(pwd)); \
	cd .. && tar czvf $(branchname)-$$(date +%Y%m%d).tgz $$here

install:
	@for dir in $(execdirs); do \
	  (cd $$dir && $(MAKE) install) || exit 1; \
	done

uninstall:
	@for dir in $(execdirs); do \
	  (cd $$dir && $(MAKE) uninstall) || exit 1; \
	done
