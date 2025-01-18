
execdirs :=   /root/cnet/logservice/ /root/cnet/gacd/ /root/cnet/glinkd/ /root/cnet/gdeliveryd/ /root/cnet/gamedbd/ /root/cnet/uniquenamed/ \
				/root/cnet/gfaction/ /root/cskill/skill/

all: libperf subs libs gs
clean: clean-libperf clean-subs clean-libs clean-gs

configure: setrules configure-shared configure-iolib
clean-configure: clean-shared clean-iolib

libs: libgs libcommon
clean-libs: clean-libgs clean-libcommon

install:
	cp -f /root/cgame/gs/gs /home/gamed/gs; \
	chmod 777 /home/gamed/gs; \
	cp -f ./cnet/gfaction/gfactiond /home/gamed/gfactiond; \
	chmod 777 /home/gamed/gfactiond

clean-install:
	rm -f /home/gamed/gs; \
	rm -f /home/gamed/gfactiond

libperf:
	@echo "Building libperf"
	$(MAKE) -C ./libperf all

clean-libperf:
	@echo "Cleaning libperf"
	$(MAKE) -C ./libperf clean

subs:
	@echo "Building subsystems"
	$(MAKE) -C ./subsystem all

clean-subs:
	@echo "Cleaning subsystems"
	$(MAKE) -C ./subsystem clean

libgs:
	@echo "Building libgs"
	$(MAKE) -C ./libgs all

clean-libgs:
	@echo "Cleaning libgs"
	$(MAKE) -C ./libgs clean

libcommon:
	@echo "Building libcommon"
	$(MAKE) -C ./libcommon all

clean-libcommon:
	@echo "Cleaning libcommon"
	$(MAKE) -C ./libcommon clean

gs:
	@echo "Building gs"
	$(MAKE) -C ./cgame/gs all

clean-gs:
	@echo "Cleaning gs"
	$(MAKE) -C ./cgame/gs clean

setrules:
	@echo "Setting rules"
	cp Make.rules.example Make.rules

configure-shared:
	@echo "Configuring shared library"
	$(MAKE) -C ./shared configure

clean-shared:
	@echo "Cleaning shared library"
	$(MAKE) -C ./shared clean

configure-iolib:
	@echo "Configuring iolib"
	$(MAKE) -C ./iolib configure

clean-iolib:
	@echo "Cleaning iolib"
	$(MAKE) -C ./iolib clean
