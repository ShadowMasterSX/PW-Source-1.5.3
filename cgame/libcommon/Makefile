include Rules.make

# Allgemeine Definitionen
BSDTHREAD = -pthread -D_REENTRANT -D_THREAD_SAFE
CFLAGS = -Wall -c $(BSDTHREAD)
INC=$(INC)
LIBS=$(CMLIB)

# Gemeinsame Objektdateien und Bibliotheken
BASEOBJ = start.o object.o grid.o netmsg.o world.o usermsg.o player.o npc.o gimp.o substance.o msgqueue.o actobject.o item.o matter.o filter.o filter_man.o attack.o item_list.o aggrolist.o actsession.o npcsession.o ainpc.o playercmd.o aipolicy.o playerctrl.o playertemplate.o npcgenerator.o terrain.o playerteam.o serviceprovider.o servicenpc.o guardnpc.o playertrade.o objmanager.o obj_interface.o potion_filter.o fly_filter.o skill_filter.o rune_filter.o playerinvade.o playertrashbox.o worldmanager.o gamelog.o gameconfig.o commonlib.o
CS2OBJ = A3DCollision.o  A3DFuncs.o  A3DGeometry.o  A3DMatrix.o  A3DQuaternion.o  BrushMan.o  BrushTree.o  CDBrush.o  ConvexHullData.o  Face.o  HalfSpace.o  NmdChd.o  NmdTree.o  SMTree.o  traceman.o
LIBOBJ = allocator.o base64.o mtime.o rwlock.o strfunc.o verbose.o astring.o crc.o parse.o spinlock.o strtok.o verifyname.o csection.o interlocked.o cmnet.o timer.o conf.o threadpool.o amemory.o arandomgen.o ASSERT.o

all: libGame libTrace libpollio libCommon

libGame: $(BASEOBJ)
	$(AR) rcs libGame.a $(BASEOBJ)

libTrace: $(CS2OBJ)
	$(AR) rcs libTrace.a $(CS2OBJ)

libpollio: io/pollio.o
	$(AR) rcs libpollio.a io/pollio.o

libCommon: $(LIBOBJ)
	$(AR) rcs libCommon.a $(LIBOBJ)

.cpp.o:
	$(CPP) -c $(CFLAGS) $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BASEOBJ) $(CS2OBJ) $(LIBOBJ) io/pollio.o libGame.a libTrace.a libpollio.a libCommon.a *.o

FORCE:
