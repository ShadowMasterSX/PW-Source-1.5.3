#BASEOBJ = start.o object.o grid.o netmsg.o world.o usermsg.o player.o npc.o gimp.o substance.o msgqueue.o actobject.o item.o matter.o filter.o filter_man.o attack.o item_list.o aggrolist.o actsession.o npcsession.o ainpc.o playercmd.o aipolicy.o playerctrl.o playertemplate.o npcgenerator.o terrain.o playerteam.o serviceprovider.o servicenpc.o guardnpc.o playertrade.o objmanager.o obj_interface.o potion_filter.o fly_filter.o skill_filter.o rune_filter.o playerinvade.o playertrashbox.o worldmanager.o invincible_filter.o sitdown_filter.o pvplimit_filter.o playerstall.o travel_filter.o gm_player.o npc_filter.o patrol_agent.o cooldown.o breath_ctrl.o player_pvp.o effect_filter.o aei_filter.o userlogin.o petnpc.o dbgprt.o aitrigger.o serverstat.o aiman.o playerduel.o duel_filter.o playerbind.o mount_filter.o petdataman.o petman.o playerbattle.o siegenpc.o amemobj.o towerbuild_filter.o global_drop.o pet_filter.o playermall.o antiwallow.o playermisc.o invisible_filter.o public_quest.o multi_exp_ctrl.o shielduser_filter.o weddingmanager.o teamrelationjob.o wedding_filter.o playerfaction.o dpsrankmanager.o playerforce.o online_award.o online_award_filter.o threadusage.o pk_protected_filter.o nonpenalty_pvp_filter.o playercountrybattle.o gt_award_filter.o global_controller.o meridianmanager.o touchtrade.o titlemanager.o playertitle.o playerdailysign.o player_reincarnation.o uniquedataclient.o historymanager.o playergiftcard.o playerbattlebase.o playertrickbattle.o autoteamman.o staticmap.o playerfatering.o fateringmanager.o playerclock.o mapresman.o random_maze.o moving_action_env.o moving_action.o playersanctuarycheck.o playerinstancereenter.o property.o
BASEOBJ = property.o playerinstancereenter.o playersanctuarycheck.o moving_action.o moving_action_env.o random_maze.o mapresman.o playerclock.o fateringmanager.o playerfatering.o staticmap.o autoteamman.o playertrickbattle.o playerbattlebase.o playergiftcard.o historymanager.o uniquedataclient.o player_reincarnation.o playerdailysign.o playertitle.o titlemanager.o touchtrade.o meridianmanager.o global_controller.o gt_award_filter.o playercountrybattle.o nonpenalty_pvp_filter.o pk_protected_filter.o threadusage.o online_award_filter.o online_award.o playerforce.o dpsrankmanager.o playerfaction.o wedding_filter.o teamrelationjob.o weddingmanager.o shielduser_filter.o multi_exp_ctrl.o public_quest.o invisible_filter.o playermisc.o antiwallow.o playermall.o pet_filter.o global_drop.o towerbuild_filter.o amemobj.o siegenpc.o playerbattle.o petman.o petdataman.o mount_filter.o playerbind.o duel_filter.o playerduel.o aiman.o serverstat.o aitrigger.o dbgprt.o petnpc.o userlogin.o aei_filter.o effect_filter.o player_pvp.o breath_ctrl.o cooldown.o patrol_agent.o npc_filter.o gm_player.o travel_filter.o playerstall.o pvplimit_filter.o sitdown_filter.o invincible_filter.o worldmanager.o playertrashbox.o playerinvade.o rune_filter.o skill_filter.o fly_filter.o potion_filter.o obj_interface.o objmanager.o playertrade.o guardnpc.o servicenpc.o serviceprovider.o playerteam.o terrain.o npcgenerator.o playertemplate.o playerctrl.o aipolicy.o playercmd.o ainpc.o npcsession.o actsession.o aggrolist.o item_list.o attack.o filter_man.o filter.o matter.o item.o actobject.o msgqueue.o substance.o gimp.o npc.o player.o usermsg.o world.o netmsg.o grid.o object.o start.o 

#GSOBJ = global_userlogin.o global_message.o global_manager.o  global_world_stubs.o 
GSOBJ = global_world_stubs.o global_manager.o global_message.o global_userlogin.o 
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