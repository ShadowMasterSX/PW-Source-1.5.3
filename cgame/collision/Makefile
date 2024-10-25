CS2OBJ = A3DCollision.o  A3DFuncs.o  A3DGeometry.o  A3DMatrix.o  A3DQuaternion.o  BrushMan.o  BrushTree.o  CDBrush.o  ConvexHullData.o  Face.o  HalfSpace.o  NmdChd.o  NmdTree.o  SMTree.o  traceman.o

all:	$(CS2OBJ)
	$(AR) libTrace.a $(CS2OBJ)
	
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
	rm -f *.o;rm -f .depend;rm -f libTrace.a 

depend: FORCE
	make dep;

include ../Rules.make

OPTIMIZE=-O2
