ALLOBJ = instance_world_message.o instance_manager.o instance_userlogin.o faction_world_message.o battleground_manager.o bg_world_message.o battleground_ctrl.o faction_world_ctrl.o countrybattle_manager.o countrybattle_world_message.o countrybattle_ctrl.o parallel_world_manager.o trickbattle_manager.o trickbattle_ctrl.o countryterritory_manager.o mnfaction_manager.o mnfaction_world_message.o mnfaction_ctrl.o

all:	$(ALLOBJ)
	
	
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
