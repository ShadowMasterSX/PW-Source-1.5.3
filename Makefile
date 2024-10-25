
execdirs :=   cnet/logservice/ cnet/gacd/ cnet/glinkd/ cnet/gdeliveryd/ cnet/gamedbd/ cnet/uniquenamed/ \
				cnet/gfaction/ cskill/skill/

all: libperf subs libs gs
clean: clean-libperf clean-subs clean-libs clean-gs

configure: setrules configure-shared configure-iolib
clean-configure: clean-shared clean-iolib

libs: libgs libcommon
clean-libs: clean-libgs clean-libcommon

install:
	cp -f ./cgame/gs/gs /pwserver/gamed/gs; \
	chmod 777 /pwserver/gamed/gs; \
	cp -f ./cnet/gfaction/gfactiond /pwserver/gfactiond/gfactiond; \
	chmod 777 /pwserver/gfactiond/gfactiond; \
	cp -f ./cnet/uniquenamed/uniquenamed /pwserver/uniquenamed/uniquenamed; \
	chmod 777 /pwserver/uniquenamed/uniquenamed; \
	cp -f ./cnet/gamedbd/gamedbd /pwserver/gamedbd; \
	chmod 777 /pwserver/gamedbd; \
	cp -f ./cnet/gdeliveryd/gdeliveryd /pwserver/gdeliveryd/gdeliveryd; \
	chmod 777 /pwserver/gdeliveryd/gdeliveryd; \
	cp -f ./cnet/glinkd/glinkd /pwserver/glinkd/glinkd; \
	chmod 777 /pwserver/glinkd/glinkd; \
	cp -f ./cnet/gacd/gacd /pwserver/gacd/gacd; \
	chmod 777 /pwserver/gacd/gacd; \
	cp -f ./cnet/logservice/logservice /pwserver/logservice/logservice; \
	chmod 777 /pwserver/logservice/logservice;

libperf:
	cd cnet/perf; \
	make;  \
	cd ../../

clean-libperf:
	cd cnet/perf; \
	make clean;  \
	cd ../../

libcommon:
	cd cgame/libcommon; \
	make -j8;  \
	cd ../../

clean-libcommon:
	cd cgame/libcommon; \
	make clean;  \
	cd ../../

libgs: libgsio libLogClient libgsPro2 libdbCli
	cd cgame/libgs; \
	mkdir -p io; \
	mkdir -p gs; \
	mkdir -p db; \
	mkdir -p sk; \
	mkdir -p log; \
	make
	cd ../../

clean-libgs: clean-libgsio clean-libLogClient clean-libgsPro2 clean-libdbCli
	cd cgame/libgs; \
	make clean; \
	cd ../../

gs:
	cd cgame; \
	make clean; \
	make -j8; \
	cd ..;

clean-gs:
	cd cgame; \
	make clean; \
	cd ..;

.PHONY: rpcgen

setrules:
	./setrules.sh;

configure-shared: clean-shared
	cd cnet; \
	ln -s ../share/common/ .; \
	ln -s ../share/io/ .; \
	ln -s ../share/perf/ .; \
	ln -s ../share/mk/ .; \
	ln -s ../share/storage/ .; \
	ln -s ../share/rpc/ .; \
	ln -s ../share/rpcgen .; \
	cd ..;

configure-iolib: clean-iolib
	mkdir -p iolib; \
	cd iolib; \
	mkdir -p inc; \
	cd inc; \
	ln -s ../../cnet/gamed/auctionsyslib.h; \
	ln -s ../../cnet/gamed/sysauctionlib.h; \
	ln -s ../../cnet/gdbclient/db_if.h; \
	ln -s ../../cnet/gamed/factionlib.h; \
	ln -s ../../cnet/common/glog.h; \
	ln -s ../../cnet/gamed/gsp_if.h; \
	ln -s ../../cnet/gamed/mailsyslib.h; \
	ln -s ../../cnet/gamed/privilege.hxx; \
	ln -s ../../cnet/gamed/sellpointlib.h; \
	ln -s ../../cnet/gamed/stocklib.h; \
	ln -s ../../cnet/gamed/webtradesyslib.h; \
	ln -s ../../cnet/gamed/kingelectionsyslib.h; \
	ln -s ../../cnet/gamed/pshopsyslib.h; \
	cd .. ; \
	ln -s ../cnet/io/libgsio.a; \
	ln -s ../cnet/gdbclient/libdbCli.a; \
	ln -s ../cnet/gamed/libgsPro2.a; \
	ln -s ../cnet/logclient/liblogCli.a; \
	ln -s ../cskill/skill/libskill.a; \
	cd ..;

rpcgen:
	cd cnet; \
	./rpcgen rpcalls.xml; \
	cd gfaction/operations; \
	./opgen.pl; \
	cd ../../..;

subs:
	for dir in $(execdirs); do \
        $(MAKE) -C $$dir clean; \
        $(MAKE) -j8 -C $$dir; \
    done

clean-subs:
	for dir in $(execdirs); do \
        $(MAKE) -C $$dir clean; \
    done

libgsio:
	cd cnet/io; \
	make lib; \
	cd ../..;

clean-libgsio:
	cd cnet/io; \
	rm -f libgsio.a; \
	cd ../..;

libLogClient:
	cd cnet/logclient; \
	make clean; \
	make -f Makefile.gs -j8; \
	cd ../..;

clean-libLogClient:
	cd cnet/logclient; \
	rm -f liblogCli.a; \
	cd ../..;

libgsPro2:
	cd cnet/gamed; \
	make clean; \
	make lib -j8; \
	cd ../..;

clean-libgsPro2:
	cd cnet/gamed; \
	make clean; \
	cd ../..;

libdbCli:
	cd cnet/gdbclient; \
	make clean; \
	make lib -j8; \
	cd ../..;

clean-libdbCli:
	cd cnet/gdbclient; \
	make clean; \
	cd ../..;

clean-shared:
	cd cnet; \
	rm -f common; \
	rm -f io; \
	rm -f perf; \
	rm -f mk; \
	rm -f storage; \
	rm -f rpc; \
	rm -f rpcgen;

clean-iolib:
	cd iolib; \
	cd inc; \
	rm -f *; \
	cd .. ;\
	rm -f lib*;
