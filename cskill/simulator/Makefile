# Makefile

TOP_SRCDIR := ..

include /root/sahre//mk/gcc.defs.mk

OBJS := simulator.o obj_interface.o substance.o mywrapper.o basicinfo.o myobject.o skillwrapper.o playerwrapper.o \
       skill.o skillstub.o skillfilter.o

OUTEROBJS := ../common/conf.o ../common/thread.o ../common/timer.o ../common/itimer.o ../io/pollio.o

INCLUDES += -I../expr -I../header -I../header/include -I../skill

DEFINES += -D_SKILL_SERVER -D_SKILL_SIMULATION

EXES := simulator

.PHONY: all clean

all: $(EXES)

skillwrapper.o: ../skill/skillwrapper.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

playerwrapper.o: ../skill/playerwrapper.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

skill.o: ../skill/skill.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

skillstub.o: ../skills/skillstub.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

skillfilter.o: ../skill/skillfilter.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

$(EXES): $(OBJS) $(OUTEROBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(OUTEROBJS) -o $(EXES)

clean:
	rm -f $(OBJS) $(EXES)

include ../mk/gcc.rules.mk
