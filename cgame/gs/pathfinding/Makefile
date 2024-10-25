PFOBJ = CompactSpacePassableOctree.o GlobalSPMap.o GlobalWaterAreaMap.o HalfSpace.o NPCChaseInWaterAgent.o NPCChaseInWaterPFAgent.o NPCChaseOnAirAgent.o NPCChaseOnAirPFAgent.o NPCChaseOnGroundAgent.o NPCDisperseChaseOnGroundAgent.o NPCFleeInWaterAgent.o NPCFleeOnAirAgent.o NPCFleeOnGroundAgent.o NPCMoveAgent.o NPCMoveMap.o NPCRambleInWaterAgent.o NPCRambleOnAirAgent.o NPCRambleOnGroundAgent.o PathFollowing.o SpatialPathFinding.o WaterAreaMap.o pathfinding.o NPCChaseSpatiallyPFAgent.o PathFinding2D.o Pf2DBfs.o NPCChaseOnGroundNoBlockAgent.o CMap.o

all:	$(PFOBJ)
	
	
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
