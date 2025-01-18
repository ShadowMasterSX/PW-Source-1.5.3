BASEOBJ = start.o object.o grid.o netmsg.o world.o usermsg.o player.o npc.o gimp.o substance.o msgqueue.o actobject.o item.o matter.o filter.o filter_man.o attack.o item_list.o aggrolist.o actsession.o npcsession.o ainpc.o playercmd.o aipolicy.o playerctrl.o playertemplate.o npcgenerator.o terrain.o playerteam.o serviceprovider.o servicenpc.o guardnpc.o playertrade.o objmanager.o obj_interface.o potion_filter.o fly_filter.o skill_filter.o rune_filter.o playerinvade.o playertrashbox.o worldmanager.o gamelog.o gameconfig.o commonlib.o

all:	$(BASEOBJ)
	$(AR) libGame.a $(BASEOBJ)

.c.o:
	$(CC) -c $(INC) $< -o $@

.cpp.o:
	$(CPP) -c $(INC) $< -o $@

clean:
	rm -f *.o libGame.a

include ../Rules.make
