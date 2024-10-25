#
# Top-Level Makefile
#

TOP_SRCDIR = .

# modules
modules := Digest-MD5-2.33.tar.gz HTML-Tagset-3.03.tar.gz MIME-Base64-3.01.tar.gz URI-1.31.tar.gz HTML-Parser-3.36.tar.gz XML-RegExp-0.03.tar.gz libxml-perl-0.07.tar.gz XML-Parser-2.34.tar.gz libnet-1.18.tar.gz libwww-perl-5.79.tar.gz XML-Grove-0.46alpha.tar.gz XML-DOM-1.43.tar.gz 

# directories
dirs := Digest-MD5-2.33 HTML-Tagset-3.03 MIME-Base64-3.01 URI-1.31 HTML-Parser-3.36 XML-RegExp-0.03 libxml-perl-0.07 XML-Parser-2.34 libnet-1.18 libwww-perl-5.79 XML-Grove-0.46alpha XML-DOM-1.43 

all: $(dirs)
	-($(foreach dir,$(dirs),$(MAKE) -C $(dir) all;))

test: all
	-($(foreach dir,$(dirs),$(MAKE) -C $(dir) test;))

clean:
	-($(foreach dir,$(dirs),$(MAKE) -C $(dir) clean;))

distclean:
	-($(foreach dir,$(dirs),rm -rf $(dir);))

install: all
	-($(foreach dir,$(dirs),$(MAKE) -C $(dir) install;))

uninstall:
	-($(foreach dir,$(dirs),$(MAKE) -C $(dir) uninstall;))

$(dirs): $($modules)
	-($(foreach module,$(modules),tar xvzf $(module);))
	-($(foreach dir,$(dirs),(cd $(dir);perl Makefile.PL;cd ..);))

