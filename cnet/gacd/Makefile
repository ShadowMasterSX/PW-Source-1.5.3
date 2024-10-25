SINGLE_THREAD = true
DEBUG_VERSION = true

TOP_SRCDIR = ..

include ../mk/gcc.defs.mk

INCLUDES += -I/usr/include/libxml2 -I/usr/include/mysql
#DEFINES += -DGACD_DEBUG_PRINT_DEBUG 

DEFINES += -DGACD_DEBUG_PRINT_ALL 

OUTEROBJS = ../io/pollio.o ../io/protocol.o ../io/security.o ../io/rpc.o ../io/proxyrpc.o ../common/octets.o ../common/thread.o ../common/conf.o ../common/timer.o ../common/itimer.o
OBJS = xmlconfig.o gaccontrolserver.o ganticheaterserver.o state.o stubs.o gacd.o

#OWNOBJS = stringhelper.o userdata.o userdatamanager.o codepiece.o codepiecelibrary.o forbiditem.o forbidcode.o forbidlibrary.o preparedcodefragment.o preparedcode.o preparedcodelibrary.o reportinfo.o timermanager.o logmanager.o punishmanager.o configmanager.o cheater.o stringcache.o usercodemanager.o codesender.o punisher.o platform.o queryproxy.o strinfo.o cpuinfo.o meminfo.o adapterinfo.o usersessiondata.o stackpattern.o patternmanager.o mouseinfo.o mempatternitem.o mempatterncode.o mempatternlibrary.o threadtime.o protostat.o debugitem.o debugcode.o debugcodelibrary.o asmcodelibrary.o codeprovider.o
#gcc after 4.7 reversed link order, so we need to flip order so static init doesnt break
OWNOBJS =  codeprovider.o asmcodelibrary.o debugcodelibrary.o debugcode.o debugitem.o protostat.o threadtime.o mempatternlibrary.o mempatterncode.o mempatternitem.o mouseinfo.o patternmanager.o stackpattern.o usersessiondata.o adapterinfo.o meminfo.o cpuinfo.o strinfo.o queryproxy.o platform.o punisher.o codesender.o usercodemanager.o stringcache.o cheater.o configmanager.o punishmanager.o logmanager.o timermanager.o reportinfo.o preparedcodelibrary.o preparedcode.o preparedcodefragment.o forbidlibrary.o forbidcode.o forbiditem.o codepiecelibrary.o codepiece.o userdatamanager.o userdata.o stringhelper.o 
CLEAN += $(OWNOBJS) $(OBJS)

all : gacd


gacd : $(OWNOBJS) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ)
#	$(LD) $(LDFLAGS) -o $@ $(OWNOBJS) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ) -lxml2  $(LIBICONV)
	$(LD) $(LDFLAGS) -o $@ $(SHARE_SOBJ) $(OUTEROBJS) $(OBJS) $(OWNOBJS)  -lxml2  $(LIBICONV)

#gacd : $(OWNOBJS) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ)
#	$(LD) $(LDFLAGS) -pthread -static -o $@ $(OWNOBJS) $(OBJS) $(OUTEROBJS) $(SHARE_SOBJ) -lxml2 -lz

myclean : clean
	rm -rf $(OWNOBJS) gacd core.* 
include ../mk/gcc.rules.mk

