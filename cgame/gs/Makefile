BASEOBJ = start.o object.o grid.o netmsg.o world.o usermsg.o player.o npc.o gimp.o substance.o msgqueue.o actobject.o item.o matter.o filter.o filter_man.o attack.o item_list.o aggrolist.o actsession.o npcsession.o ainpc.o playercmd.o aipolicy.o playerctrl.o playertemplate.o npcgenerator.o terrain.o playerteam.o serviceprovider.o servicenpc.o guardnpc.o playertrade.o objmanager.o obj_interface.o potion_filter.o fly_filter.o skill_filter.o rune_filter.o playerinvade.o playertrashbox.o worldmanager.o  invincible_filter.o sitdown_filter.o pvplimit_filter.o playerstall.o travel_filter.o gm_player.o npc_filter.o patrol_agent.o cooldown.o breath_ctrl.o player_pvp.o effect_filter.o aei_filter.o userlogin.o petnpc.o dbgprt.o aitrigger.o serverstat.o aiman.o playerduel.o duel_filter.o playerbind.o mount_filter.o petdataman.o petman.o playerbattle.o siegenpc.o amemobj.o towerbuild_filter.o global_drop.o pet_filter.o playermall.o antiwallow.o playermisc.o invisible_filter.o public_quest.o multi_exp_ctrl.o shielduser_filter.o weddingmanager.o teamrelationjob.o wedding_filter.o playerfaction.o dpsrankmanager.o playerforce.o online_award.o online_award_filter.o threadusage.o pk_protected_filter.o nonpenalty_pvp_filter.o playercountrybattle.o gt_award_filter.o global_controller.o meridianmanager.o touchtrade.o titlemanager.o playertitle.o playerdailysign.o player_reincarnation.o uniquedataclient.o historymanager.o playergiftcard.o playerbattlebase.o playertrickbattle.o autoteamman.o staticmap.o playerfatering.o fateringmanager.o playerclock.o mapresman.o random_maze.o moving_action_env.o moving_action.o playersanctuarycheck.o playerinstancereenter.o playersolochallenge.o playermnfaction.o

GSOBJ = global_userlogin.o global_message.o global_manager.o  global_world_stubs.o 
INOBJ = instance/*.o 

SUBDIR = io item template task  pathfinding  instance ai wallow mobile
SUBLIB = usercmd/libusercmd.a
IOOBJ = io/msgio.o
ITEMOBJ = item/*.o
WALLOWOBJ = wallow/*.o
TEMPLATEOBJ = template/*.o
TASKOBJ = task/taskman.o 
PFOBJ = pathfinding/*.o
AIOBJ = ai/*.o
TASKOBJ2 = task/TaskProcess.o  task/TaskTempl.o  task/TaskTemplMan.o task/TaskServer.o
MOBILEOBJ = mobile/*.o
SUBOBJ= $(IOOBJ) $(ITEMOBJ) $(TEMPLATEOBJ) $(TASKOBJ) $(PFOBJ) $(AIOBJ) $(WALLOWOBJ) $(MOBILEOBJ)

all: 	gs

gs: $(BASEOBJ) $(GSOBJ) $(SUBDIR) 
#	$(LD) -fPIC $(BASEOBJ) $(INOBJ) $(GSOBJ) $(SUBOBJ) $(CMLIB) libtask.so $(ALLLIB) -o gs 
	$(LD) -fPIC $(SUBOBJ) $(GSOBJ) $(INOBJ) $(BASEOBJ) libtask.so libskill.so $(CMLIB) $(ALLLIB) -o gs 

test:	$(BASEOBJ) $(GSOBJ) $(SUBDIR) 
	$(AR) libtgs.a $(BASEOBJ) $(INOBJ) $(GSOBJ) $(SUBOBJ) $(CMLIB) 

$(SUBDIR): FORCE
	cd $@ ;make -j8

lib: $(IOLIB)
	cd ..; make gslib

solib: libtask.so

libtask.so: task
	cd task; make lib -j8

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
	rm -f *.o *.a;rm -f gs; 
	-($(foreach dir,$(SUBDIR),$(MAKE) -C $(dir) clean;))

depend: FORCE
	make dep;
	-($(foreach dir,$(SUBDIR),$(MAKE) -C $(dir) dep;))

include ../Rules.make
INC += -I/usr/local/ssl/include