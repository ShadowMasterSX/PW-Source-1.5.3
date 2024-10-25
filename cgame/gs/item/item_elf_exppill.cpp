#include "../clstab.h"
#include "../world.h"
#include "../player_imp.h"
#include "item_elf_exppill.h"

DEFINE_SUBSTANCE(elf_exppill_item, item_body, CLS_ITEM_ELF_EXPPILL)		//CLS在clstab.h中定义

bool elf_exppill_item::Load(archive & ar)
{ 
	ar.pop_back(&ess, sizeof(ess));
	if(ess.exp <= 0 || ess.level <= 0 || ess.level > player_template::GetMaxLevel()) return false; 
	CalcCRC();
	return true; 
};

int elf_exppill_item::OnUse(item::LOCATION l, int index, gactive_imp * imp, const char * arg, size_t arg_size)
{
	ASSERT(l == item::INVENTORY && arg_size == sizeof(int));
	gplayer_imp * pImp = (gplayer_imp *)imp;
	
	unsigned int useexp = *(unsigned int *)arg;
	if(useexp <= 0 || useexp > ess.exp)
	{
		imp->_runner->error_message(S2C::ERR_ELF_INSERT_EXP_USE_PILL_FAILED);
		return -1;
	}
	//这里假定只能给装备了的小精灵注经验
	unsigned int _useexp = pImp->ElfInsertExpUsePill(useexp, ess.level);
	
	if(_useexp == (unsigned int)-1)
	{
		return -1;
	}
	ess.exp -= _useexp;
	
	if(ess.exp <= 0)
		return 1;	//表示使用完了一个经验丸
	else
	{
		CalcCRC();
		pImp->PlayerGetItemInfo(gplayer_imp::IL_INVENTORY, index);
		return 0;
	}	
}

