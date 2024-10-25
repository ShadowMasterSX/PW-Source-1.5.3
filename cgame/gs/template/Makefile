MSGOBJ = elementdataman.o setclassid.o npcgendata.o itemdataman.o el_precinct.o city_region.o el_region.o a3dmatrix.o sevbezier.o pathman.o globaldataman.o extra_drop_table.o el_randommapinfo.o

all:	$(MSGOBJ)
	
	
.c.o:
	$(CC) -c $(INC) $< -o $@

.cc.o:
	$(CPP) -c $(INC) $< -o $@

.cxx.o:
	$(CPP) -c $(INC) $< -o $@ 

.cpp.o:
	$(CPP) -c $(INC) $< -o $@ 

FORCE:

clean: 	FORCE
	rm -f *.o;rm -f .depend 

depend: FORCE
	make dep;

include ../../Rules.make
