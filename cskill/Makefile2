#
# Makefile 
#

TOP_SRCDIR = ..

include ./mk/gcc.defs.mk

SHAREOBJ = 
LOGSTUB = 
LOGOBJ = 

OBJS = ./skill/skill.o ./skill/skillwrapper.o ./skill/playerwrapper.o ./skill/skillfilter.o

STUBOBJS =  $(./skills/stubs*.o)

CLEAN += ./skills/*.o 

INCLUDES += -I./skill -I./header -I./header/include
DEFINES	+=-D_SKILL_SERVER 
CFLAGS += -fPIC #-finput-charset="cp1251"

all : dlib

dlib : $(OBJS) $(STUBOBJS) 
	rm libskill.so -f
	$(LD) -shared -fPIC $(LDFLAGS) $(OBJS) $(STUBOBJS) -o libskill.so


include ./mk/gcc.rules.mk


	
	