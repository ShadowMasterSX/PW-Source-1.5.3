MSGOBJ = TaskProcess.o TaskTempl.o TaskTemplMan.o TaskServer.o taskman.o
SOOBJ = TaskProcess.o TaskTempl.o TaskTemplMan.o TaskServer.o

all:	$(MSGOBJ)

lib: 	$(SOOBJ)
	$(LD) -shared -fPIC $(SOOBJ) -o ../libtask.so
	
.c.o:
	$(CC) -fPIC -c $(INC) $< -o $@

.cc.o:
	$(CPP) -fPIC -c $(INC) $< -o $@

.cxx.o:
	$(CPP) -fPIC -c $(INC) $< -o $@ 

.cpp.o:
	$(CPP) -fPIC -c $(INC) $< -o $@ 

FORCE:

clean: 	FORCE
	rm -f *.o *.so;rm -f .depend 

depend: FORCE
	make dep;

include ../../Rules.make
