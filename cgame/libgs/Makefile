
LIB_PATH=$(BASEPATH)/libgs

all: FORCE clean
	cd gs;ar x $(IOPATH)/libgsPro2.a 
	cd io;ar x $(IOPATH)/libgsio.a 
	cd db;ar x $(IOPATH)/libdbCli.a 
	cd log;ar x $(IOPATH)/liblogCli.a

FORCE:

clean: FORCE
	rm -f *.o;rm -f .lib.o.list
	cd io;rm -f *.o
	cd gs;rm -f *.o
	cd db;rm -f *.o
	cd log;rm -f *.o
	

include ../Rules.make
