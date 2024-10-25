#include "../clstab.h"
#include "../world.h"
#include "../actobject.h"
#include "../clstab.h"
#include "fashion_item.h"
#include "../worldmanager.h"

DEFINE_SUBSTANCE(fashion_item,item_body, CLS_ITEM_FASHION_ITEM)

bool 
fashion_item::VerifyRequirement(item_list & list,gactive_imp* obj)
{ 
	int class_limit = world_manager::GetDataMan().get_item_class_limit(_tid);

	if(class_limit && !((1 << (obj->GetObjectClass() & 0x0F)) & class_limit))
	{
		__PRINTINFO("读取时装[%d]职业限制[%d]\n",_tid,class_limit);
		return false;
	}

	bool isFemale = ((gactive_object*)obj->_parent)->IsFemale();
	return _ess.ess.require_level <= obj->GetHistoricalMaxLevel() && ((!_ess.ess.gender &&  !isFemale) || (_ess.ess.gender && isFemale));
}

int 
fashion_item::GetIdModify()
{	
	return _color_mask;
}

void 
fashion_item::InitFromShop() 
{
    DATA_TYPE dt;
    FASHION_ESSENCE* pTemplateEss = (FASHION_ESSENCE *)world_manager::GetDataMan().get_data_ptr(_tid, ID_SPACE_ESSENCE, dt);
    if(dt != DT_FASHION_ESSENCE || pTemplateEss == NULL) return;

    unsigned short color = 0;
    if(pTemplateEss->combined_switch & FCS_RANDOM_COLOR_IN_RANGE)
    {
        float h,s,v;
        h = abase::Rand(pTemplateEss->h_min, pTemplateEss->h_max);
		s = abase::Rand((float)pow(pTemplateEss->s_min,2), (float)pow(pTemplateEss->s_max,2));
		v = abase::Rand((float)pow(pTemplateEss->v_min,3), (float)pow(pTemplateEss->v_max,3));
		
		s = sqrtf(s);
		v = pow(v,1.0/3); 
		
		if(s > pTemplateEss->s_max) s = pTemplateEss->s_max; else if(s < pTemplateEss->s_min) s = pTemplateEss->s_min;
		if(v > pTemplateEss->v_max) v = pTemplateEss->v_max; else if(v < pTemplateEss->v_min) v = pTemplateEss->v_min;
		
        int color_tmp = hsv2rgb(h,s,v);

        unsigned short r = ((color_tmp >> 16) >> 3) & 0x1F;
        unsigned short g = ((color_tmp >> 8) >> 3) & 0x1F;
        unsigned short b = (color_tmp >> 3) & 0x1F;
        color = ((r << 10) | (g << 5) | b) & 0x7FFF;
    }
    else
    {
        unsigned short r = abase::Rand(0,(1<<5) -1);
		unsigned short g = abase::Rand(0,(1<<5) -1);
		unsigned short b = abase::Rand(0,(1<<5) -1);
        color = ((r << 10) | (g << 5) | b) & 0x7FFF;
    }

    _ess.ess.color = color;
    _color_mask = (_ess.ess.color << 16);
}

