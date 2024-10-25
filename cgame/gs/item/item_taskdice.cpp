#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_taskdice.h"
#include "../task/taskman.h"
#include "../player_imp.h"

DEFINE_SUBSTANCE(item_taskdice,item_body,CLS_ITEM_TASKDICE)

int
item_taskdice::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	ASSERT(obj->GetRunTimeClass()->IsDerivedFrom(CLASSINFO(gplayer_imp)));
	__PRINTF("ʹ�������񴥷���Ʒ�� ��ͼ��������%d\n",_ess.task_id);

	if(obj->IsCombatState())
	{
		DATA_TYPE dt;
		struct TASKDICE_ESSENCE* ess = (struct TASKDICE_ESSENCE*)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
		if(ess == NULL || dt != DT_TASKDICE_ESSENCE)
		{
			ASSERT(false);
			return -1;
		}
		if(ess->no_use_in_combat)
		{
			obj->_runner->error_message(S2C::ERR_INVALID_OPERATION_IN_COMBAT);
			return -1;
		}
	}
	
	PlayerTaskInterface  task_if((gplayer_imp*)obj);
	if(OnTaskCheckDeliver(&task_if,_ess.task_id,0))
	{
		__PRINTF("��������ɹ�\n");
		return 1;
	}
	obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
	return -1;
}

