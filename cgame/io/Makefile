include Rules.make

# Allgemeine Definitionen
BSDTHREAD = -pthread -D_REENTRANT -D_THREAD_SAFE
CFLAGS = -Wall -c $(BSDTHREAD)
INC=$(INC)
LIBS=$(CMLIB)

# Gemeinsame Objektdateien und Bibliotheken
BASEOBJ = start.o object.o grid.o netmsg.o world.o usermsg.o player.o npc.o gimp.o substance.o msgqueue.o actobject.o item.o matter.o filter.o filter_man.o attack.o item_list.o aggrolist.o actsession.o npcsession.o ainpc.o playercmd.o aipolicy.o playerctrl.o playertemplate.o npcgenerator.o terrain.o playerteam.o serviceprovider.o servicenpc.o guardnpc.o playertrade.o objmanager.o obj_interface.o potion_filter.o fly_filter.o skill_filter.o rune_filter.o playerinvade.o playertrashbox.o worldmanager.o gamelog.o gameconfig.o commonlib.o
CS2OBJ = A3DCollision.o  A3DFuncs.o  A3DGeometry.o  A3DMatrix.o  A3DQuaternion.o  BrushMan.o  BrushTree.o  CDBrush.o  ConvexHullData.o  Face.o  HalfSpace.o  NmdChd.o  NmdTree.o  SMTree.o  traceman.o
LIBOBJ = io/pollio.o

all: libGame libTrace libpollio

libGame: $(BASEOBJ)
	$(AR) rcs libGame.a $(BASEOBJ)

libTrace: $(CS2OBJ)
	$(AR) rcs libTrace.a $(CS2OBJ)

libpollio: $(LIBOBJ)
	$(AR) rcs libpollio.a $(LIBOBJ)

.cpp.o:
	$(CPP) -c $(CFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BASEOBJ) $(CS2OBJ) $(LIBOBJ) libGame.a libTrace.a libpollio.a *.o

FORCE:
