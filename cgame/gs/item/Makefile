MSGOBJ = equip_item.o item_stone.o item_addon.o item_potion.o  item_flysword.o item_tossmatter.o item_townscroll.o item_rune.o item_taskdice.o fashion_item.o item_skilltome.o item_effect.o item_savescroll.o item_mobgen.o item_petegg.o item_petfood.o item_control.o item_fireworks.o item_skilltrigger.o item_dummy.o item_bugle.o item_bible.o item_amulet.o item_dbexp.o item_opentrash.o item_elf.o item_elf_exppill.o item_elf_equip.o item_stallcard.o item_skilltrigger2.o item_queryotherproperty.o item_incskillability.o item_wedding.o item_sharpener.o item_congregate.o item_forceticket.o item_dynskill.o set_addon.o item_generalcard.o generalcard_set_man.o item_generalcard_dice.o item_soul.o item_astrolabe.o item_occuppackage.o item_fixpositiontransmit.o

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
