#
# Top-Level Makefile
#

TOP_SRCDIR = .

branchname =dbserver

# directories
execdirs := common/ io/ storage/
extradirs :=
docdirs :=
alldirs := $(docdirs) $(extradirs) $(execdirs)
tardirs := $(alldirs)

# rules
.PHONY: $(alldirs) clean distclean tar doctar dist

all: $(execdirs)

clean:
	-($(foreach dir,$(alldirs),$(MAKE) -C $(dir) clean;))

distclean:
	-($(foreach dir,$(alldirs),$(MAKE) -C $(dir) distclean;))

#kdocs: 
#	-($(foreach dir,$(docdirs),$(MAKE) -C $(dir) kdocs;))

tar: distclean
	( here=`pwd`; here=`basename $$here`; cd ..; tar czvf $(branchname)-`date +%Y%m%d`.tgz $$here )

install:
	@for dir in ${execdirs}; do \
	  (cd $$dir && $(MAKE) install) \
	  || case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

uninstall:
	@for dir in ${execdirs}; do \
	  (cd $$dir && $(MAKE) uninstall) \
	  || case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"

#doctar: kdocs
#	(here=`pwd`; here=`basename $$here`; cd ..; tar czvf $(branchname)-docs-`date +%Y%m%d`.tgz $$here/*.html $$here/graphics $(docfiles))

#dist: tar doctar

$(alldirs):
	$(MAKE) -C $(@D) $(@F) all

