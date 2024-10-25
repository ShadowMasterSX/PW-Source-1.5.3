#include "../clstab.h"
#include "../world.h"
#include "../common/message.h"
#include "../worldmanager.h"
#include "../actobject.h"
#include "../item_list.h"
#include "item_soul.h"

DEFINE_SUBSTANCE(soul_item,item_body,CLS_ITEM_SOUL)

int
soul_item::OnUse(item::LOCATION l,gactive_imp * obj,size_t count)
{
	DATA_TYPE dt;
	const MONSTER_SPIRIT_ESSENCE & ess = *(const MONSTER_SPIRIT_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid,ID_SPACE_ESSENCE,dt);
	ASSERT(&ess && dt == DT_MONSTER_SPIRIT_ESSENCE);

	if (!obj->UseSoulItem(ess.type, ess.level, ess.power))
	{
		obj->_runner->error_message(S2C::ERR_CANNOT_USE_ITEM);
		return -1;
	}
	return 1;
}
