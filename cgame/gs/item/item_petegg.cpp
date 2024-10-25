#include "../world.h"
#include "item_petegg.h"
#include "../clstab.h"
#include "../worldmanager.h"

DEFINE_SUBSTANCE(item_pet_egg,item_body,CLS_ITEM_PET_EGG)

bool 
item_pet_egg::Load(archive & ar)
{
	pe_essence pe;
	ar.pop_back(&pe,sizeof(pe));
	
	//宠物蛋职业限制从模板中读取
	int class_limit	= world_manager::GetDataMan().get_item_class_limit(pe.pet_tid);
	if(class_limit && class_limit != pe.require_class)
		pe.require_class = class_limit;
	
	if(pe.pet_class != 5)
	{
		_size = sizeof(pe) + pe.skill_count * sizeof(int) * 2;
	}
	else
	{
		_size = sizeof(pe) + pe.skill_count * sizeof(int)*2 + sizeof(evo_prop);
	}
	_ess = (pe_essence *) abase::fastalloc(_size);
	memcpy(_ess,&pe,sizeof(pe));
	ar.pop_back(((char*)(_ess)) + sizeof(pe), _size - sizeof(pe));
	return true;
}

int
item_pet_egg::OnUse(item::LOCATION l,int index,gactive_imp * pImp,size_t count)
{
	//检查职业级别限制
	if(!((1<<(pImp->GetObjectClass() & 0x1F)) & _ess->require_class)) 
	{
		pImp->_runner->error_message(S2C::ERR_INVALID_PLAYER_CALSS);
		return -1;
	}
	
	if(pImp->GetHistoricalMaxLevel() < _ess->require_level || pImp->GetHistoricalMaxLevel() < _ess->level) 
	{
		pImp->_runner->error_message(S2C::ERR_LEVEL_NOT_MATCH);
		return -1;
	}

	if(pImp->AddPetToSlot(_ess,index))
	{
		return 1;
	}
	return -1;
}

