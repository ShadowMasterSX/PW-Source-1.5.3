#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <algorithm>
#include "../config.h"
#include "../world.h"
#include "item_addon.h"
#include "equip_item.h"
#include <arandomgen.h>
#include "../actobject.h"
#include "../template/elementdataman.h"
#include "../player_imp.h"
#include "../cooldowncfg.h"


namespace 
{
	template <typename ARG_TYPE> class arg_addon;

	inline int GenPointData2(addon_data & data)
	{
		return abase::RandNormal(data.arg[0] , data.arg[1]);
	}

	inline int GenPercentData2_O(addon_data & data)
	{
		float p0 = *(float*)&(data.arg[0]);
		float p1 = *(float*)&(data.arg[1]);
		ASSERT(p1 >= p0);
		p0 = abase::RandNormal((int)(p0 * 100.f+0.1f), (int)(p1 * 100.f+0.1f));
		return *(int*)&p0;
	}

	inline int GenSecondData2(addon_data & data)
	{
		float p0 = *(float*)&(data.arg[0]);
		float p1 = *(float*)&(data.arg[1]);
		ASSERT(p1 >= p0);
		p0 = abase::RandNormal((int)(p0 * 20.f+0.1f), (int)(p1 * 20.f+0.1f));
		return *(int*)&p0;
	}

	struct PERCENT {};
	struct POINT {};
	struct SECOND {};

	struct DOUBLE_PERCENT {};
	struct DOUBLE_POINT {};
	struct DOUBLE_SECOND {};
	struct DOUBLE_FIX_POINT {};

	struct TRIPLE_POINT {};

	template <>
	class arg_addon<POINT> : public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 1);
			return 1;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 1);
		}
	};

	template <>
	class arg_addon<DOUBLE_FIX_POINT> : public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 2);
			return 2;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 2);
		}
	};
	
	
	template <>
	class arg_addon <PERCENT>: public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 1);
			float p = *(float*)&(data.arg[0]);
			data.arg[0] = (int)(p * 100.f + 0.1f);
			return 1;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 1);
		}
	};

	template <>
	class arg_addon<SECOND> : public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 1);
			float p = *(float*)&(data.arg[0]);
			data.arg[0] = (int)(p * 20.f + 0.1f);
			return 1;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 1);
		}
	};

	template <>
	class arg_addon<DOUBLE_POINT> : public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 2);
			data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
			return 1;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 1);
		}
	};
	
	
	template <>
	class arg_addon <DOUBLE_PERCENT>: public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 2);
			float p0 = *(float*)&(data.arg[0]);
			float p1 = *(float*)&(data.arg[1]);
			ASSERT(p1 >= p0);
			data.arg[0] = abase::RandNormal((int)(p0 * 100.f + 0.1f), (int)(p1 * 100.f + 0.1f));
			return 1;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 1);
		}
	};

	template <>
	class arg_addon<DOUBLE_SECOND> : public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 2);
			float p0 = *(float*)&(data.arg[0]);
			float p1 = *(float*)&(data.arg[1]);
			ASSERT(p1 >= p0);
			data.arg[0] = abase::RandNormal((int)(p0 * 20.f), (int)(p1 * 20.f));
			return 1;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 1);
		}
	};

	template <>
	class arg_addon<TRIPLE_POINT> : public addon_handler
	{
	public:
		virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
		{
			ASSERT(arg_num == 3);
			data.arg[0] = abase::RandNormal(data.arg[0] - data.arg[2], data.arg[0] + data.arg[2]);
			data.arg[1] = abase::RandNormal(data.arg[1] - data.arg[2], data.arg[1] + data.arg[2]);
			return 2;
		}

		virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
		{
			return 0;
		}
		inline void Check(const addon_data & data)
		{
			//由于精炼属性的存在，此检查作废
			//ASSERT(addon_manager::GetArgCount(data.id) == 2);
		}
	};
	enum
	{
		POINT_OFF = offsetof(gactive_imp, _en_point),
		PERCENT_OFF = offsetof(gactive_imp, _en_percent),
		EQ_POINT = offsetof(equip_item, _base_param),
		EQ_PERCENT = offsetof(equip_item, _base_param_percent),
		
		//POINT_OFF = (((size_t)&(((gactive_imp*)100)->_en_point)) - 100),
		//PERCENT_OFF = ((size_t)&(((gactive_imp*)100)->_en_percent)) - 100,
		//EQ_POINT = ((size_t)&(((equip_item*)100)->_base_param)) - 100,
		//EQ_PERCENT = ((size_t)&(((equip_item*)100)->_base_param_percent)) - 100,
	};
#define MY_OFFSETOF(st,member) offsetof(st, member)
//(((size_t)&(((st*)100)->member)) - 100)
}

class enhance_durability_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype,addon_data & data,int argnum)
	{
		ASSERT(argnum == 1);
		return 1;
	}
	
	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
	{ 
		float enhance = 1 + *(float*)&data.arg[0];
		require->durability = (int)(require->durability * enhance);
		require->max_durability = (int)(require->max_durability * enhance);
		return 0;
	}

};


class enhance_durability_addon_point : public essence_addon
{
public:
	virtual int GenerateParam(int datatype,addon_data & data,int argnum)
	{
		ASSERT(argnum == 1);
		return 1;
	}
	
	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
	{ 
		require->durability 	+= data.arg[0];
		require->max_durability += data.arg[0];
		return 0;
	}
};

class reduce_require_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype,addon_data & data,int arg_num)
	{
		ASSERT(arg_num == 2);
		float p0 = *(float*)&(data.arg[0]);
		float p1 = *(float*)&(data.arg[1]);
		ASSERT(p1 >= p0);
		p0 = abase::RandNormal(p0 ,p1 );
		data.arg[0] = *(int*)&p0;
		return 1;
	}
	
	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t size,prerequisition * require)
	{ 
		float reduce = *(float*)&data.arg[0];
		ASSERT(reduce > 0 && reduce < 1.f);
		reduce = 1.f - reduce;
		require->strength = (int)(require->strength * reduce);
		require->agility = (int)(require->agility * reduce);
		require->energy = (int)(require->energy * reduce);
		require->vitality = (int)(require->vitality * reduce);
		return 0;
	}
};

class enhance_speed_addon: public arg_addon<PERCENT>		// enhance param (single arg) addon
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		Check(data);
		pImp->_en_percent.walk_speed += data.arg[0];
		pImp->_en_percent.run_speed += data.arg[0];
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_en_percent.walk_speed -= data.arg[0];
		pImp->_en_percent.run_speed -= data.arg[0];
		return 0;
	}
};

class enhance_speed_addon_point: public arg_addon<POINT>		// enhance param (single arg) addon
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		Check(data);
		pImp->_en_point.walk_speed += *(float*)&(data.arg[0]);
		pImp->_en_point.run_speed += *(float*)&(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_en_point.walk_speed -= *(float*)&(data.arg[0]);
		pImp->_en_point.run_speed -= *(float*)&(data.arg[0]);
		return 0;
	}
};

template <typename PARAM_TYPE, int OFFSET_IN_IMP,typename ARGTYPE>
class EPSA_addon: public arg_addon<ARGTYPE>		// enhance param (single arg) addon
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	inline void Check(const addon_data & data)
	{
		//由于精炼属性的存在，此检查作废
		//ASSERT(addon_manager::GetArgCount(data.id) == 1);
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		Check(data);
		*(PARAM_TYPE*)((char*)pImp + OFFSET_IN_IMP) += *(PARAM_TYPE*)&(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		*(PARAM_TYPE*)((char*)pImp + OFFSET_IN_IMP) -= *(PARAM_TYPE*)&(data.arg[0]);
		return 0;
	}
};

template <typename PARAM_TYPE, int OFFSET_IN_IMP, int OFFSET_OUT_IMP,typename ARGTYPE>
class EPSA_addon_spec: public arg_addon<ARGTYPE>		// enhance param (single arg) addon
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}
	
	inline void Check(const addon_data & data)
	{
		//由于精炼属性的存在，此检查作废
		//ASSERT(addon_manager::GetArgCount(data.id) == 1);
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		Check(data);
		*(PARAM_TYPE*)((char*)pImp + OFFSET_IN_IMP) += *(PARAM_TYPE*)&(data.arg[0]);
		*(PARAM_TYPE*)((char*)pImp + OFFSET_OUT_IMP) -= *(PARAM_TYPE*)&(data.arg[1]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		*(PARAM_TYPE*)((char*)pImp + OFFSET_IN_IMP) -= *(PARAM_TYPE*)&(data.arg[0]);
		*(PARAM_TYPE*)((char*)pImp + OFFSET_OUT_IMP) += *(PARAM_TYPE*)&(data.arg[1]);
		return 0;
	}
};

template <int OFFSET_IN_IMP,typename ARGTYPE>
class EPSA_EQ_addon: public arg_addon<ARGTYPE>		// enhance param (single arg) addon
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		*(int*)((char*)equip + OFFSET_IN_IMP) += data.arg[0];
		return addon_manager::ADDON_MASK_STATIC;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_STATIC;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		ASSERT(false);
		return 0;
	}
};

template <typename ARG_TYPE>
class template_enhance_damage: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		equip->_base_param.damage_low += data.arg[0];
		equip->_base_param.damage_high += data.arg[0];
		return addon_manager::ADDON_MASK_STATIC;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_STATIC;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp){ASSERT(false);return 0;}
	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) {ASSERT(false);return 0;}
};
typedef template_enhance_damage<POINT>			enhance_damage_addon;
typedef template_enhance_damage<DOUBLE_POINT>	enhance_damage_addon_2arg;

template <typename ARG_TYPE>
class template_enhance_magic_damage: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		equip->_base_param.magic_damage_low += data.arg[0];
		equip->_base_param.magic_damage_high += data.arg[0];
		return addon_manager::ADDON_MASK_STATIC;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_STATIC;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp){ASSERT(false);return 0;}
	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) {ASSERT(false);return 0;}
};
typedef template_enhance_magic_damage<POINT>		enhance_magic_damage_addon;
typedef template_enhance_magic_damage<DOUBLE_POINT>	enhance_magic_damage_addon_2arg;

template <typename ARG_TYPE>
class template_enhance_damage_2: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp)
	{
		pImp->_en_point.damage_low += data.arg[0];
		pImp->_en_point.damage_high += data.arg[0];
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) 
	{
		pImp->_en_point.damage_low -= data.arg[0];
		pImp->_en_point.damage_high -= data.arg[0];
		return 0;
	}
};
typedef template_enhance_damage_2<POINT>			enhance_damage_addon_2;
typedef template_enhance_damage_2<DOUBLE_POINT>		enhance_damage_addon_2_2arg;

template <typename ARG_TYPE>
class template_enhance_magic_damage_2: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp)
	{
		pImp->_en_point.magic_dmg_low += data.arg[0];
		pImp->_en_point.magic_dmg_high += data.arg[0];
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) 
	{
		pImp->_en_point.magic_dmg_low -= data.arg[0];
		pImp->_en_point.magic_dmg_high -= data.arg[0];
		return 0;
	}
};
typedef template_enhance_magic_damage_2<POINT>			enhance_magic_damage_addon_2;
typedef template_enhance_magic_damage_2<DOUBLE_POINT>	enhance_magic_damage_addon_2_2arg;

template <typename ARG_TYPE>
class template_enhance_all_resistance: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp)
	{
		pImp->EnhanceAllResistance(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) 
	{
		pImp->ImpairAllResistance(data.arg[0]);
		return 0;
	}
};
typedef template_enhance_all_resistance<POINT>			enhance_all_resistance_addon;
typedef template_enhance_all_resistance<DOUBLE_POINT>	enhance_all_resistance_addon_2arg;

template <typename ARG_TYPE>
class template_enhance_all_resistance_scale: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp)
	{
		pImp->EnhanceScaleAllResistance(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) 
	{
		pImp->ImpairScaleAllResistance(data.arg[0]);
		return 0;
	}
};
typedef template_enhance_all_resistance_scale<PERCENT>			enhance_all_resistance_scale_addon;
typedef template_enhance_all_resistance_scale<DOUBLE_PERCENT>	enhance_all_resistance_scale_addon_2arg;

template <typename ARG_TYPE>
class template_enhance_all_magic_damage_reduce: public arg_addon<ARG_TYPE>
{
public:
	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *) { ASSERT(false); return 0; } 
	virtual int Activate(const addon_data & data, equip_item * item, gactive_imp *pImp)
	{
		pImp->EnhanceAllMagicDamageReduce(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp) 
	{
		pImp->ImpairAllMagicDamageReduce(data.arg[0]);
		return 0;
	}
};
typedef template_enhance_all_magic_damage_reduce<PERCENT> enhance_all_magic_damage_reduce_addon;
typedef template_enhance_all_magic_damage_reduce<DOUBLE_PERCENT> enhance_all_magic_damage_reduce_addon_2arg;

class item_addon_random : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		return 0;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		return 0;
	}

	virtual bool IsRandomAddon() 
	{ 
		return true;
	}
};

class enhance_attack_range_addon_2arg : public addon_handler
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(arg_num == 2);
		*(float*)&data.arg[0] = abase::RandNormal(*(float*)&data.arg[0] , *(float*)&data.arg[1]);
		return 1;
	}
	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require){return 0;}
	virtual int UpdateItem(const addon_data & , equip_item *){return addon_manager::ADDON_MASK_ACTIVATE;}
	virtual int TestUpdate(){return addon_manager::ADDON_MASK_ACTIVATE;}
	virtual int Use(const addon_data & , equip_item *, gactive_imp *){ASSERT(false);return 0;}
	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->_en_point.attack_range += *(float*)&data.arg[0];
		return 0;
	}
	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_en_point.attack_range -= *(float*)&data.arg[0];
		return 0;
	}
};

class enhance_attack_speed_addon : public addon_handler
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		float speed = *(float*)&(data.arg[0]);
		ASSERT(speed >= -1 && speed <= 1);
		if(speed <= -1 || speed> 1) return -1;
		data.arg[0] = (int)(speed * 20);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		return 0;
	}

	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->_en_point.attack_speed -= data.arg[0];
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_en_point.attack_speed += data.arg[0];
		return 0;
	}
};

template <typename ARG_TYPE>
class template_reduce_cast_time : public arg_addon <ARG_TYPE> 
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		int num = arg_addon <ARG_TYPE>::GenerateParam(datatype,data,arg_num);
		ASSERT(data.arg[0] >= -100 && data.arg[0] <= 100);
		return num;
	}

	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->_skill.DecPrayTime(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_skill.IncPrayTime(data.arg[0]);
		return 0;
	}
};
typedef template_reduce_cast_time<PERCENT>			reduce_cast_time_addon;
typedef template_reduce_cast_time<DOUBLE_PERCENT>	reduce_cast_time_addon_2arg;

class item_skill_addon : public addon_handler
{
public:
	//尚未实现
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(arg_num == 2);
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		return 0;
	}

	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->_skill.ActivateSkill(pImp,data.arg[0],data.arg[1]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_skill.DeactivateSkill(pImp,data.arg[0],data.arg[1]);
		return 0;
	}
};

class item_rebound_skill_addon : public addon_handler
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(arg_num == 3);
		return 3;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		return 0;
	}

	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->_skill.ActivateReboundSkill(pImp,data.arg[0],data.arg[1],data.arg[2]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_skill.DeactivateReboundSkill(pImp,data.arg[0],data.arg[1],data.arg[2]);
		return 0;
	}
};



class query_other_property_addon : public addon_handler
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(arg_num == 1);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		return 0;
	}

	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_USE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_USE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp * obj)
	{
		gplayer_imp * pImp = (gplayer_imp*)obj;
		gplayer * pPlayer = (gplayer*)(pImp->_parent);
		//物品使用冷却
		if(!pImp->CheckCoolDown(COOLDOWN_INDEX_QUERY_OTHER_PROPERTY)) 
		{
			pImp->_runner->error_message(S2C::ERR_OBJECT_IS_COOLING);
			return -1;
		}
		XID target = pImp->GetCurTarget();
		if(!target.IsPlayer() || target.id == pPlayer->ID.id)
			return -1;
		
		world::object_info info;
		if(!pImp->_plane->QueryObject(target,info)) return -1;
		//A3DVECTOR pos = pPlayer->pos;
		//float dis = pos.squared_distance(info.pos);
		//if(dis > 100.f*100.f) return -1;
		
		pImp->SendTo<0>(GM_MSG_QUERY_PROPERTY,target,
				-1,NULL,0);

		pImp->SetCoolDown(COOLDOWN_INDEX_QUERY_OTHER_PROPERTY,QUERY_OTHER_PROPERTY_COOLDOWN_TIME);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		ASSERT(false);
		return 0;
	}
};

//同时提高物理防御力和法术防御力
class enhance_defense_resistance_addon : public arg_addon<DOUBLE_FIX_POINT>
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}
	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->_en_point.defense += data.arg[0];
		pImp->EnhanceAllResistance(data.arg[1]);
		return 0;
	}
	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->_en_point.defense -= data.arg[0];
		pImp->ImpairAllResistance(data.arg[1]);
		return 0;
	}
};

class enhance_soulpower_addon: public arg_addon<POINT>				  
{
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->EnhanceSoulPower(data.arg[0]);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->ImpairSoulPower(data.arg[0]);
		return 0;
	}
};

class enhance_mount_speed_addon : public addon_handler
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		float speed = *(float*)&(data.arg[0]);
		ASSERT(speed >= 0);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		return 0;
	}

	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		pImp->EnhanceMountSpeedEn( *(float*)&(data.arg[0]));
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *, gactive_imp *pImp)
	{
		pImp->ImpairMountSpeedEn( *(float*)&(data.arg[0]));
		return 0;
	}
};

template <int critical_value,typename BASE_HANDLER>
class  set_equip_addon : public BASE_HANDLER
{
public:
	typedef BASE_HANDLER PARENT;
public:
	virtual int UpdateItem(const addon_data & , equip_item *)
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}
	virtual int TestUpdate()
	{
		return addon_manager::ADDON_MASK_ACTIVATE;
	}

	virtual int Use(const addon_data & , equip_item *, gactive_imp *)
	{
		ASSERT(false);
		return 0;
	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		if(pImp->ActivateSetAddon(data.id) == critical_value)
		{
			PARENT::Activate(data,item,pImp);
		}
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item * item, gactive_imp *pImp)
	{
		if(pImp->DeactivateSetAddon(data.id) == (critical_value - 1))
		{
			PARENT::Deactivate(data,item,pImp);
		}
		return 0;
	}
};

template <typename BASE_ADDON>
class refine_addon_template : public BASE_ADDON
{
public:
	virtual int GenerateParam(int datatype, addon_data & data, int arg_num)
	{
		int rst = BASE_ADDON::GenerateParam(datatype,data,arg_num);
		ASSERT(rst == 1);
		data.arg[1] = 0;
		return 2;
	}

//	virtual int OnRefineData(addon_data & srcdata,const addon_data & newdata, bool reverse)
//	{
//		if(reverse)
//		{
//			srcdata.arg[0] -= newdata.arg[0];
//			srcdata.arg[1] --;
//		}
//		else
//		{
//			srcdata.arg[0] += newdata.arg[0];
//			srcdata.arg[1] ++;
//		}
//		return 0;
//	}

	
};

template <typename BASE_ADDON1, typename BASE_ADDON2>
class refine_addon_template2 : public BASE_ADDON1
{
	BASE_ADDON2 _de;
public:
	virtual int GenerateParam(int datatype, addon_data & data, int arg_num)
	{
		int rst = BASE_ADDON1::GenerateParam(datatype,data,arg_num);
		ASSERT(rst == 1);
		data.arg[1] = 0;
		return 2;
	}

	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		int rst1 = BASE_ADDON1::UpdateItem(data,equip);
		int rst2 = _de.UpdateItem(data,equip);
		ASSERT(rst1 == rst2);
		return rst1;
	}

//	virtual int OnRefineData(addon_data & srcdata,const addon_data & newdata, bool reverse)
//	{
//		if(reverse)
//		{
//			srcdata.arg[0] -= newdata.arg[0];
//			srcdata.arg[1] --;
//		}
//		else
//		{
//			srcdata.arg[0] += newdata.arg[0];
//			srcdata.arg[1] ++;
//		}
//		return 0;
//	}

	virtual int Activate(const addon_data & data , equip_item * item, gactive_imp *pImp)
	{
		BASE_ADDON1::Activate(data,item,pImp);
		_de.Activate(data,item,pImp);
		return 0;
	}

	virtual int Deactivate(const addon_data & data, equip_item *item, gactive_imp *pImp)
	{
		BASE_ADDON1::Deactivate(data,item,pImp);
		_de.Deactivate(data,item,pImp);
		return 0;
	}

};

//具有时效的附加属性
template <typename BASE_ADDON>
class temporary_addon_template : public BASE_ADDON
{
public:
	virtual int GenerateParam(int datatype, addon_data & data, int arg_num)
	{
		int rst = BASE_ADDON::GenerateParam(datatype,data,arg_num);
		ASSERT(rst == 1);
		data.arg[1] = 0xFFFF;		//过期的时间点, 该值必须大于0
		return 2;
	}
	
	virtual int GetExpireDate(const addon_data & data){ return data.arg[1]; }
};

typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,hp_gen),POINT> enhance_hpgen_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,mp_gen),POINT> enhance_mpgen_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,hp_gen),DOUBLE_POINT> enhance_hpgen_addon_2arg;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,mp_gen),DOUBLE_POINT> enhance_mpgen_addon_2arg;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,hp_gen),DOUBLE_PERCENT> enhance_hpgen_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,mp_gen),DOUBLE_PERCENT> enhance_mpgen_scale_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,max_hp),POINT> enhance_hp_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,max_mp),POINT> enhance_mp_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,max_hp),DOUBLE_POINT> enhance_hp_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,max_mp),DOUBLE_POINT> enhance_mp_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,attack),POINT> enhance_attack_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,attack),DOUBLE_POINT> enhance_attack_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,defense),POINT> enhance_defense_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,defense),DOUBLE_POINT> enhance_defense_addon_2_2arg;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,armor),POINT> enhance_armor_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,armor),DOUBLE_POINT> enhance_armor_addon_2_2arg;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,max_hp),PERCENT> enhance_hp_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,max_mp),PERCENT> enhance_mp_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,attack),PERCENT> enhance_attack_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,attack),DOUBLE_PERCENT> enhance_attack_scale_addon_2arg;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,damage),PERCENT> enhance_damage_scale_addon_2;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,magic_dmg),PERCENT> enhance_magic_damage_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,defense),PERCENT> enhance_defense_scale_addon_2;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,defense),DOUBLE_PERCENT> enhance_defense_scale_addon_2_2arg;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,resistance[0]),PERCENT> enhance_resistance0_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,resistance[1]),PERCENT> enhance_resistance1_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,resistance[2]),PERCENT> enhance_resistance2_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,resistance[3]),PERCENT> enhance_resistance3_scale_addon;
typedef EPSA_addon<int, PERCENT_OFF+offsetof(scale_enhanced_param,resistance[4]),PERCENT> enhance_resistance4_scale_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,resistance[0]),POINT> enhance_resistance0_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,resistance[1]),POINT> enhance_resistance1_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,resistance[2]),POINT> enhance_resistance2_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,resistance[3]),POINT> enhance_resistance3_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,resistance[4]),POINT> enhance_resistance4_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_damage_reduce),PERCENT> enhance_damage_reduce_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_damage_reduce),DOUBLE_PERCENT> enhance_damage_reduce_addon_2arg;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_magic_damage_reduce[0]),PERCENT> enhance_magic_damage_reduce0_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_magic_damage_reduce[1]),PERCENT> enhance_magic_damage_reduce1_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_magic_damage_reduce[2]),PERCENT> enhance_magic_damage_reduce2_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_magic_damage_reduce[3]),PERCENT> enhance_magic_damage_reduce3_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_magic_damage_reduce[4]),PERCENT> enhance_magic_damage_reduce4_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_crit_rate),PERCENT> enhance_crit_rate;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_crit_rate),DOUBLE_PERCENT> enhance_crit_rate_2arg;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_exp_addon),PERCENT> enhance_exp_addon;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_attack_degree),POINT> enhance_attack_degree;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_defend_degree),POINT> enhance_defend_degree;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_attack_degree),DOUBLE_POINT> enhance_attack_degree_2arg;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_defend_degree),DOUBLE_POINT> enhance_defend_degree_2arg;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_penetration),POINT> enhance_penetration;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_resilience),POINT> enhance_resilience;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_penetration),DOUBLE_POINT> enhance_penetration_2arg;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_resilience),DOUBLE_POINT> enhance_resilience_2arg;
typedef EPSA_addon<int, MY_OFFSETOF(gactive_imp,_vigour_en),POINT> enhance_vigour;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,damage_high),POINT> enhance_max_damage_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,magic_dmg_high),POINT> enhance_max_magic_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,damage_high),DOUBLE_POINT> enhance_max_damage_addon_2_2arg;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,magic_dmg_high),DOUBLE_POINT> enhance_max_magic_addon_2_2arg;

typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,armor),DOUBLE_PERCENT> enhance_armor_scale_addon;
typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,armor),PERCENT> enhance_armor_scale_addon_single;
typedef EPSA_EQ_addon<EQ_POINT+offsetof(equip_item::base_data,armor),POINT> enhance_armor_addon;
typedef EPSA_EQ_addon<EQ_POINT+offsetof(equip_item::base_data,armor),DOUBLE_POINT> enhance_armor_range_addon;
typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,defense),DOUBLE_PERCENT> enhance_defense_scale_addon;
typedef EPSA_EQ_addon<EQ_POINT+offsetof(equip_item::base_data,defense),DOUBLE_POINT> enhance_defense_addon;
typedef EPSA_EQ_addon<EQ_POINT+offsetof(equip_item::base_data,defense),POINT> enhance_defense_addon_1arg;

typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,damage),PERCENT> enhance_damage_scale_addon;
typedef EPSA_EQ_addon<EQ_POINT+offsetof(equip_item::base_data,damage_high),POINT> enhance_max_damage_addon;
typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,magic_damage),PERCENT> enhance_magic_scale_addon;
typedef EPSA_EQ_addon<EQ_POINT+offsetof(equip_item::base_data,magic_damage_high),POINT> enhance_max_magic_addon;

typedef EPSA_addon_spec<int, POINT_OFF+offsetof(enhanced_param,str), POINT_OFF+offsetof(enhanced_param,agi),DOUBLE_FIX_POINT> enhance_str_addon2;

typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,str), POINT> enhance_str_addon_1arg;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,agi), POINT> enhance_agi_addon_1arg;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,vit), POINT> enhance_vit_addon_1arg;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,eng), POINT> enhance_eng_addon_1arg;

typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,str), DOUBLE_POINT> enhance_str_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,agi), DOUBLE_POINT> enhance_agi_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,vit), DOUBLE_POINT> enhance_vit_addon;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,eng), DOUBLE_POINT> enhance_eng_addon;

typedef refine_addon_template<enhance_all_resistance_addon> refine_resistance;
typedef refine_addon_template<enhance_armor_addon> refine_armor;
typedef refine_addon_template<enhance_defense_addon_1arg> refine_defense;
typedef refine_addon_template<enhance_hp_addon> refine_max_hp;
typedef refine_addon_template2<enhance_magic_damage_addon,enhance_damage_addon> refine_magic_damage;
typedef refine_addon_template<enhance_damage_addon> refine_damage;
typedef refine_addon_template2<enhance_defense_addon_2,enhance_all_resistance_addon> refine_defense_resistance;

#include "item_addon_all.h"
#include "item_addon_def.h"

#define MY_INSERT_ADDON(type) INSERT_ADDON(type,IA_##type)
int addon_inserter::_counter = 0;
bool InitAllAddon()
{
	INSERT_ADDON(216,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(217,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(218,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(219,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(220,IA_EA_ESS<offsetof(armor_essence,defense)>);

	INSERT_ADDON(221,enhance_defense_scale_addon);
	INSERT_ADDON(222,enhance_defense_scale_addon);
	INSERT_ADDON(223,enhance_defense_scale_addon);
	INSERT_ADDON(224,enhance_defense_scale_addon);
	INSERT_ADDON(225,enhance_defense_scale_addon);

	INSERT_ADDON(226,item_armor_enhance_resistance<0>);
	INSERT_ADDON(227,item_armor_enhance_resistance<0>);
	INSERT_ADDON(228,item_armor_enhance_resistance<0>);
	INSERT_ADDON(229,item_armor_enhance_resistance<0>);
	INSERT_ADDON(230,item_armor_enhance_resistance<0>);

	INSERT_ADDON(231,item_armor_scale_enhance_resistance<0>);
	INSERT_ADDON(232,item_armor_scale_enhance_resistance<0>);
	INSERT_ADDON(233,item_armor_scale_enhance_resistance<0>);
	INSERT_ADDON(234,item_armor_scale_enhance_resistance<0>);
	INSERT_ADDON(235,item_armor_scale_enhance_resistance<0>);

	INSERT_ADDON(236,item_armor_enhance_resistance<1>);
	INSERT_ADDON(237,item_armor_enhance_resistance<1>);
	INSERT_ADDON(238,item_armor_enhance_resistance<1>);
	INSERT_ADDON(239,item_armor_enhance_resistance<1>);
	INSERT_ADDON(240,item_armor_enhance_resistance<1>);

	INSERT_ADDON(424,item_armor_scale_enhance_resistance<1>);
	INSERT_ADDON(425,item_armor_scale_enhance_resistance<1>);
	INSERT_ADDON(426,item_armor_scale_enhance_resistance<1>);
	INSERT_ADDON(427,item_armor_scale_enhance_resistance<1>);
	INSERT_ADDON(428,item_armor_scale_enhance_resistance<1>);

	INSERT_ADDON(429,item_armor_enhance_resistance<2>);
	INSERT_ADDON(430,item_armor_enhance_resistance<2>);
	INSERT_ADDON(431,item_armor_enhance_resistance<2>);
	INSERT_ADDON(432,item_armor_enhance_resistance<2>);
	INSERT_ADDON(433,item_armor_enhance_resistance<2>);

	INSERT_ADDON(241,item_armor_scale_enhance_resistance<2>);
	INSERT_ADDON(242,item_armor_scale_enhance_resistance<2>);
	INSERT_ADDON(243,item_armor_scale_enhance_resistance<2>);
	INSERT_ADDON(244,item_armor_scale_enhance_resistance<2>);
	INSERT_ADDON(245,item_armor_scale_enhance_resistance<2>);

	INSERT_ADDON(246,item_armor_enhance_resistance<3>);
	INSERT_ADDON(247,item_armor_enhance_resistance<3>);
	INSERT_ADDON(248,item_armor_enhance_resistance<3>);
	INSERT_ADDON(249,item_armor_enhance_resistance<3>);
	INSERT_ADDON(250,item_armor_enhance_resistance<3>);

	INSERT_ADDON(251,item_armor_scale_enhance_resistance<3>);
	INSERT_ADDON(252,item_armor_scale_enhance_resistance<3>);
	INSERT_ADDON(253,item_armor_scale_enhance_resistance<3>);
	INSERT_ADDON(254,item_armor_scale_enhance_resistance<3>);
	INSERT_ADDON(255,item_armor_scale_enhance_resistance<3>);

	INSERT_ADDON(256,item_armor_enhance_resistance<4>);
	INSERT_ADDON(257,item_armor_enhance_resistance<4>);
	INSERT_ADDON(258,item_armor_enhance_resistance<4>);
	INSERT_ADDON(259,item_armor_enhance_resistance<4>);
	INSERT_ADDON(260,item_armor_enhance_resistance<4>);

	INSERT_ADDON(261,item_armor_scale_enhance_resistance<4>);
	INSERT_ADDON(262,item_armor_scale_enhance_resistance<4>);
	INSERT_ADDON(263,item_armor_scale_enhance_resistance<4>);
	INSERT_ADDON(264,item_armor_scale_enhance_resistance<4>);
	INSERT_ADDON(265,item_armor_scale_enhance_resistance<4>);

	INSERT_ADDON(266,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(267,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(268,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(269,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(270,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 

	INSERT_ADDON(271,IA_EA_ESS_SCALE<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(272,IA_EA_ESS_SCALE<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(273,IA_EA_ESS_SCALE<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(274,IA_EA_ESS_SCALE<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(275,IA_EA_ESS_SCALE<offsetof(armor_essence,hp_enhance)>);

	INSERT_ADDON(276,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(277,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(278,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(279,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(280,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 

	INSERT_ADDON(281,IA_EA_ESS_SCALE<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(282,IA_EA_ESS_SCALE<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(283,IA_EA_ESS_SCALE<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(284,IA_EA_ESS_SCALE<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(285,IA_EA_ESS_SCALE<offsetof(armor_essence,mp_enhance)>);

	INSERT_ADDON(286, enhance_speed_addon);
	INSERT_ADDON(287, enhance_speed_addon);
	INSERT_ADDON(288, enhance_speed_addon);
	INSERT_ADDON(289, enhance_speed_addon);
	INSERT_ADDON(290, enhance_speed_addon);

	INSERT_ADDON(291,enhance_armor_scale_addon);
	INSERT_ADDON(292,enhance_armor_scale_addon);
	INSERT_ADDON(293,enhance_armor_scale_addon);
	INSERT_ADDON(294,enhance_armor_scale_addon);
	INSERT_ADDON(295,enhance_armor_scale_addon);

	INSERT_ADDON(296, enhance_durability_addon);
	INSERT_ADDON(297, enhance_durability_addon);
	INSERT_ADDON(298, enhance_durability_addon);
	INSERT_ADDON(299, enhance_durability_addon);
	INSERT_ADDON(300, enhance_durability_addon);

	INSERT_ADDON(301, reduce_require_addon);
	INSERT_ADDON(302, reduce_require_addon);
	INSERT_ADDON(303, reduce_require_addon);
	INSERT_ADDON(304, reduce_require_addon);
	INSERT_ADDON(305, reduce_require_addon);

	INSERT_ADDON(306, item_armor_enhance_all_resistance);
	INSERT_ADDON(307, item_armor_enhance_all_resistance);
	INSERT_ADDON(308, item_armor_enhance_all_resistance);
	INSERT_ADDON(309, item_armor_enhance_all_resistance);
	INSERT_ADDON(310, item_armor_enhance_all_resistance);

	INSERT_ADDON(311, enhance_damage_reduce_addon);
	INSERT_ADDON(312, enhance_damage_reduce_addon);
	INSERT_ADDON(313, enhance_damage_reduce_addon);
	INSERT_ADDON(314, enhance_damage_reduce_addon);
	INSERT_ADDON(315, enhance_damage_reduce_addon);

	INSERT_ADDON(316, item_armor_specific_addon);
	INSERT_ADDON(317, item_armor_specific_addon);
	INSERT_ADDON(318, item_armor_specific_addon);
	INSERT_ADDON(319, item_armor_specific_addon);
	INSERT_ADDON(320, item_armor_specific_addon);

	INSERT_ADDON(321, enhance_hpgen_addon);
	INSERT_ADDON(322, enhance_hpgen_addon);
	INSERT_ADDON(323, enhance_hpgen_addon);
	INSERT_ADDON(324, enhance_hpgen_addon);
	INSERT_ADDON(325, enhance_hpgen_addon);

	INSERT_ADDON(326, enhance_mpgen_addon);
	INSERT_ADDON(327, enhance_mpgen_addon);
	INSERT_ADDON(328, enhance_mpgen_addon);
	INSERT_ADDON(329, enhance_mpgen_addon);
	INSERT_ADDON(330, enhance_mpgen_addon);

	INSERT_ADDON(336, item_addon_random); 

	INSERT_ADDON(331, enhance_attack_speed_addon);
	INSERT_ADDON(337, enhance_attack_speed_addon);
	INSERT_ADDON(338, enhance_attack_speed_addon);
	INSERT_ADDON(339, enhance_attack_speed_addon);

	INSERT_ADDON(332, reduce_cast_time_addon);
	INSERT_ADDON(333, reduce_cast_time_addon);
	INSERT_ADDON(334, reduce_cast_time_addon);
	INSERT_ADDON(335, reduce_cast_time_addon);

	INSERT_ADDON(341, IA_ED_ESS<offsetof(decoration_essence,damage)>); 
	INSERT_ADDON(342, IA_ED_ESS<offsetof(decoration_essence,damage)>); 
	INSERT_ADDON(343, IA_ED_ESS<offsetof(decoration_essence,damage)>); 

	INSERT_ADDON(344, item_decoration_scale_enhance_damage);

	INSERT_ADDON(345, item_decoration_specific_damage_addon);
	INSERT_ADDON(346, item_decoration_specific_damage_addon);
	INSERT_ADDON(347, item_decoration_specific_damage_addon);

	INSERT_ADDON(348, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(349, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(350, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);

	INSERT_ADDON(351, item_decoration_scale_enhance_magic_damage);

	INSERT_ADDON(352, item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(353, item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(354, item_decoration_specific_magic_damage_addon);

	INSERT_ADDON(355, enhance_attack_speed_addon);	//$$$$$$$$$$$$ 魔法咏唱时间减少，未实现

	INSERT_ADDON(356, enhance_crit_rate);
	INSERT_ADDON(357, enhance_crit_rate);
	INSERT_ADDON(358, enhance_crit_rate);

	INSERT_ADDON(359, IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(360, IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(361, IA_ED_ESS<offsetof(decoration_essence,defense)>);

	INSERT_ADDON(362, enhance_damage_reduce_addon);
	INSERT_ADDON(363, enhance_damage_reduce_addon);
	INSERT_ADDON(364, enhance_damage_reduce_addon);

	INSERT_ADDON(365, item_decoration_enchance_resistance<0>);
	INSERT_ADDON(366, item_decoration_enchance_resistance<0>);
	INSERT_ADDON(367, item_decoration_enchance_resistance<0>);
	INSERT_ADDON(368, item_decoration_enchance_resistance<1>);
	INSERT_ADDON(369, item_decoration_enchance_resistance<1>);
	INSERT_ADDON(370, item_decoration_enchance_resistance<1>);
	INSERT_ADDON(371, item_decoration_enchance_resistance<2>);
	INSERT_ADDON(372, item_decoration_enchance_resistance<2>);
	INSERT_ADDON(373, item_decoration_enchance_resistance<2>);
	INSERT_ADDON(374, item_decoration_enchance_resistance<3>);
	INSERT_ADDON(375, item_decoration_enchance_resistance<3>);
	INSERT_ADDON(376, item_decoration_enchance_resistance<3>);
	INSERT_ADDON(377, item_decoration_enchance_resistance<4>);
	INSERT_ADDON(378, item_decoration_enchance_resistance<4>);
	INSERT_ADDON(379, item_decoration_enchance_resistance<4>);

	INSERT_ADDON(380, item_decoration_enhance_all_resistance);
	INSERT_ADDON(381, item_decoration_enhance_all_resistance);
	INSERT_ADDON(382, item_decoration_enhance_all_resistance);


	INSERT_ADDON(383, enhance_hpgen_addon);
	INSERT_ADDON(384, enhance_mpgen_addon);

	INSERT_ADDON(385, empty_addon);			//$$$$$$$$$$$$$ 组队属性没有想好
	INSERT_ADDON(386, empty_addon);			//$$$$$$$$$$$$$ 组队属性没有想好

	INSERT_ADDON(387, enhance_hp_scale_addon);
	INSERT_ADDON(388, enhance_mp_scale_addon);

	INSERT_ADDON(389, enhance_attack_scale_addon);
	INSERT_ADDON(390, enhance_attack_scale_addon);
	INSERT_ADDON(391, enhance_attack_scale_addon);

	INSERT_ADDON(392, enhance_armor_scale_addon_single);
	INSERT_ADDON(393, enhance_armor_scale_addon_single);
	INSERT_ADDON(394, enhance_armor_scale_addon_single);
	
	INSERT_ADDON(403, empty_addon);			//$$$$$$$$$$$$$ 组队属性没有想好

	INSERT_ADDON(404, enhance_exp_addon);
	INSERT_ADDON(405, enhance_exp_addon);
	INSERT_ADDON(406, enhance_exp_addon);

	INSERT_ADDON(407, enhance_durability_addon);
	INSERT_ADDON(408, enhance_durability_addon);
	INSERT_ADDON(409, enhance_durability_addon);

	INSERT_ADDON(410, item_addon_random); 


	INSERT_ADDON(411, enhance_weapon_damage_addon);
	INSERT_ADDON(413, enhance_weapon_damage_addon);
	INSERT_ADDON(415, enhance_weapon_damage_addon);

	INSERT_ADDON(412, enhance_weapon_max_damage_addon);
	INSERT_ADDON(414, enhance_weapon_max_damage_addon);
	INSERT_ADDON(416, enhance_weapon_max_damage_addon);

	INSERT_ADDON(417, enhance_damage_scale_addon);
	INSERT_ADDON(418, enhance_damage_scale_addon);
	INSERT_ADDON(419, enhance_damage_scale_addon);

	INSERT_ADDON(420, enhance_weapon_speed_addon);
	INSERT_ADDON(421, enhance_weapon_speed_addon);

	INSERT_ADDON(422, enhance_weapon_magic_addon);
	INSERT_ADDON(434, enhance_weapon_magic_addon);
	INSERT_ADDON(436, enhance_weapon_magic_addon);

	INSERT_ADDON(423, enhance_weapon_max_magic_addon);
	INSERT_ADDON(435, enhance_weapon_max_magic_addon);
	INSERT_ADDON(437, enhance_weapon_max_magic_addon);

	INSERT_ADDON(438, enhance_magic_scale_addon);
	INSERT_ADDON(439, enhance_magic_scale_addon);
	INSERT_ADDON(440, enhance_magic_scale_addon);

	INSERT_ADDON(441, reduce_cast_time_addon);
	INSERT_ADDON(442, reduce_cast_time_addon);
	INSERT_ADDON(443, reduce_cast_time_addon);
	INSERT_ADDON(444, reduce_cast_time_addon);

	INSERT_ADDON(445, item_skill_addon);
	INSERT_ADDON(446, item_skill_addon);
	INSERT_ADDON(447, item_skill_addon);
	INSERT_ADDON(448, item_skill_addon);
	INSERT_ADDON(449, item_skill_addon);
	INSERT_ADDON(450, item_skill_addon);
	INSERT_ADDON(451, item_skill_addon);
	INSERT_ADDON(452, item_skill_addon);
	INSERT_ADDON(453, item_skill_addon);
	INSERT_ADDON(454, item_skill_addon);
	INSERT_ADDON(455, item_skill_addon);
	INSERT_ADDON(456, item_skill_addon);
	INSERT_ADDON(457, item_skill_addon);
	INSERT_ADDON(458, item_skill_addon);
	INSERT_ADDON(459, item_skill_addon);
	INSERT_ADDON(460, item_skill_addon);
	INSERT_ADDON(461, item_skill_addon);

	INSERT_ADDON(462, enhance_attack_scale_addon);
	INSERT_ADDON(463, enhance_attack_scale_addon);
	INSERT_ADDON(464, enhance_attack_scale_addon);

	INSERT_ADDON(465, enhance_durability_addon);
	INSERT_ADDON(467, enhance_durability_addon);
	INSERT_ADDON(469, enhance_durability_addon);

	INSERT_ADDON(466, reduce_require_addon);
	INSERT_ADDON(468, reduce_require_addon);
	INSERT_ADDON(470, reduce_require_addon);

	INSERT_ADDON(471, enhance_weapon_attack_range);
	
	INSERT_ADDON(472, item_addon_random); 

	INSERT_ADDON(473, enhance_crit_rate);
	INSERT_ADDON(474, enhance_crit_rate);
	INSERT_ADDON(475, enhance_crit_rate);

	INSERT_ADDON(476,enhance_damage_addon);
	INSERT_ADDON(477,enhance_defense_addon_1arg);
	INSERT_ADDON(478,enhance_damage_addon);
	INSERT_ADDON(479,enhance_defense_addon_1arg);
	INSERT_ADDON(481,enhance_damage_addon);
	INSERT_ADDON(482,enhance_defense_addon_1arg);
	INSERT_ADDON(553,enhance_damage_addon);
	INSERT_ADDON(557,enhance_defense_addon_1arg);
	INSERT_ADDON(554,enhance_damage_addon);
	INSERT_ADDON(558,enhance_defense_addon_1arg);
	INSERT_ADDON(555,enhance_damage_addon);
	INSERT_ADDON(559,enhance_defense_addon_1arg);
	INSERT_ADDON(556,enhance_damage_addon);
	INSERT_ADDON(560,enhance_defense_addon_1arg);
	INSERT_ADDON(1144,enhance_damage_addon);
	INSERT_ADDON(1145,enhance_defense_addon_1arg);
	INSERT_ADDON(1146,enhance_damage_addon);
	INSERT_ADDON(1147,enhance_defense_addon_1arg);
	INSERT_ADDON(1148,enhance_damage_addon);
	INSERT_ADDON(1149,enhance_defense_addon_1arg);
	INSERT_ADDON(1150,enhance_damage_addon);
	INSERT_ADDON(1151,enhance_defense_addon_1arg);
	INSERT_ADDON(1152,enhance_damage_addon);
	INSERT_ADDON(1153,enhance_defense_addon_1arg);

	INSERT_ADDON(483,enhance_magic_damage_addon);
	INSERT_ADDON(484,enhance_all_resistance_addon);
	INSERT_ADDON(485,enhance_magic_damage_addon);
	INSERT_ADDON(486,enhance_all_resistance_addon);
	INSERT_ADDON(488,enhance_magic_damage_addon);
	INSERT_ADDON(489,enhance_all_resistance_addon);

	INSERT_ADDON(561,enhance_magic_damage_addon);
	INSERT_ADDON(565,enhance_all_resistance_addon);
	INSERT_ADDON(562,enhance_magic_damage_addon);
	INSERT_ADDON(566,enhance_all_resistance_addon);
	INSERT_ADDON(563,enhance_magic_damage_addon);
	INSERT_ADDON(567,enhance_all_resistance_addon);
	INSERT_ADDON(564,enhance_magic_damage_addon);
	INSERT_ADDON(568,enhance_all_resistance_addon);
	INSERT_ADDON(1154,enhance_magic_damage_addon);
	INSERT_ADDON(1155,enhance_all_resistance_addon);
	INSERT_ADDON(1156,enhance_magic_damage_addon);
	INSERT_ADDON(1157,enhance_all_resistance_addon);
	INSERT_ADDON(1158,enhance_magic_damage_addon);
	INSERT_ADDON(1159,enhance_all_resistance_addon);
	INSERT_ADDON(1160,enhance_magic_damage_addon);
	INSERT_ADDON(1161,enhance_all_resistance_addon);
	INSERT_ADDON(1162,enhance_magic_damage_addon);
	INSERT_ADDON(1163,enhance_all_resistance_addon);

	INSERT_ADDON(490,enhance_attack_addon);
	INSERT_ADDON(491,enhance_armor_addon);
	INSERT_ADDON(492,enhance_attack_addon);
	INSERT_ADDON(493,enhance_armor_addon);
	INSERT_ADDON(494,enhance_attack_addon);
	INSERT_ADDON(495,enhance_armor_addon);
	INSERT_ADDON(569,enhance_attack_addon);
	INSERT_ADDON(573,enhance_armor_addon);
	INSERT_ADDON(570,enhance_attack_addon);
	INSERT_ADDON(574,enhance_armor_addon);
	INSERT_ADDON(571,enhance_attack_addon);
	INSERT_ADDON(575,enhance_armor_addon);
	INSERT_ADDON(572,enhance_attack_addon);
	INSERT_ADDON(576,enhance_armor_addon);
	INSERT_ADDON(1164,enhance_attack_addon);
	INSERT_ADDON(1165,enhance_armor_addon);
	INSERT_ADDON(1166,enhance_attack_addon);
	INSERT_ADDON(1167,enhance_armor_addon);
	INSERT_ADDON(1168,enhance_attack_addon);
	INSERT_ADDON(1169,enhance_armor_addon);
	INSERT_ADDON(1170,enhance_attack_addon);
	INSERT_ADDON(1171,enhance_armor_addon);
	INSERT_ADDON(1172,enhance_attack_addon);
	INSERT_ADDON(1173,enhance_armor_addon);

	INSERT_ADDON(497,enhance_hp_addon);
	INSERT_ADDON(498,enhance_hp_addon);
	INSERT_ADDON(499,enhance_hp_addon);
	INSERT_ADDON(577,enhance_hp_addon);
	INSERT_ADDON(578,enhance_hp_addon);
	INSERT_ADDON(579,enhance_hp_addon);
	INSERT_ADDON(580,enhance_hp_addon);
	INSERT_ADDON(1174,enhance_hp_addon);
	INSERT_ADDON(1175,enhance_hp_addon);
	INSERT_ADDON(1176,enhance_hp_addon);
	INSERT_ADDON(1177,enhance_hp_addon);
	INSERT_ADDON(1178,enhance_hp_addon);

	INSERT_ADDON(500,enhance_mp_addon);
	INSERT_ADDON(501,enhance_mp_addon);
	INSERT_ADDON(502,enhance_mp_addon);
	INSERT_ADDON(581,enhance_mp_addon);
	INSERT_ADDON(582,enhance_mp_addon);
	INSERT_ADDON(583,enhance_mp_addon);
	INSERT_ADDON(584,enhance_mp_addon);
	INSERT_ADDON(1179,enhance_mp_addon);
	INSERT_ADDON(1180,enhance_mp_addon);
	INSERT_ADDON(1181,enhance_mp_addon);
	INSERT_ADDON(1182,enhance_mp_addon);
	INSERT_ADDON(1183,enhance_mp_addon);

#define IDMRA(x,y) item_decoration_magic_resistance_addon<x,y>
	INSERT_ADDON(512,IDMRA(0,3));
	INSERT_ADDON(513,IDMRA(1,0));
	INSERT_ADDON(514,IDMRA(2,4));
	INSERT_ADDON(515,IDMRA(3,2));
	INSERT_ADDON(516,IDMRA(4,1));
	INSERT_ADDON(517,IDMRA(0,3));
	INSERT_ADDON(518,IDMRA(1,0));
	INSERT_ADDON(519,IDMRA(2,4));
	INSERT_ADDON(520,IDMRA(3,2));
	INSERT_ADDON(521,IDMRA(4,1));
	INSERT_ADDON(522,IDMRA(0,3));
	INSERT_ADDON(523,IDMRA(1,0));
	INSERT_ADDON(524,IDMRA(2,4));
	INSERT_ADDON(525,IDMRA(3,2));
	INSERT_ADDON(526,IDMRA(4,1));

#define IAERA(x,y) item_armor_enhance_resistance_addon<x,y>
	INSERT_ADDON(528,IAERA(0,3));
	INSERT_ADDON(527,IAERA(0,3));
	INSERT_ADDON(529,IAERA(0,3));
	INSERT_ADDON(530,IAERA(0,3));
	INSERT_ADDON(531,IAERA(0,3));
	
	INSERT_ADDON(532,IAERA(1,0));
	INSERT_ADDON(533,IAERA(1,0));
	INSERT_ADDON(534,IAERA(1,0));
	INSERT_ADDON(535,IAERA(1,0));
	INSERT_ADDON(536,IAERA(1,0));
	
	INSERT_ADDON(537,IAERA(2,4));
	INSERT_ADDON(538,IAERA(2,4));
	INSERT_ADDON(539,IAERA(2,4));
	INSERT_ADDON(540,IAERA(2,4));
	INSERT_ADDON(541,IAERA(2,4));
	
	INSERT_ADDON(542,IAERA(3,2));
	INSERT_ADDON(543,IAERA(3,2));
	INSERT_ADDON(544,IAERA(3,2));
	INSERT_ADDON(545,IAERA(3,2));
	INSERT_ADDON(546,IAERA(3,2));
	
	INSERT_ADDON(547,IAERA(4,1));
	INSERT_ADDON(548,IAERA(4,1));
	INSERT_ADDON(549,IAERA(4,1));
	INSERT_ADDON(550,IAERA(4,1));
	INSERT_ADDON(551,IAERA(4,1));

	INSERT_ADDON(590,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(598,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(599,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(600,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(601,IA_EA_ESS<offsetof(armor_essence,defense)>);

#define IAERA2(x,y) item_armor_enhance_resistance_addon_2<x,y>
	INSERT_ADDON(592,IAERA2(0,3));
	INSERT_ADDON(602,IAERA2(0,3));
	INSERT_ADDON(603,IAERA2(0,3));
	INSERT_ADDON(604,IAERA2(0,3));
	INSERT_ADDON(605,IAERA2(0,3));
	             
	INSERT_ADDON(606,IAERA2(1,0));
	INSERT_ADDON(607,IAERA2(1,0));
	INSERT_ADDON(608,IAERA2(1,0));
	INSERT_ADDON(609,IAERA2(1,0));
	INSERT_ADDON(610,IAERA2(1,0));
	             
	INSERT_ADDON(611,IAERA2(2,4));
	INSERT_ADDON(612,IAERA2(2,4));
	INSERT_ADDON(613,IAERA2(2,4));
	INSERT_ADDON(614,IAERA2(2,4));
	INSERT_ADDON(615,IAERA2(2,4));
	             
	INSERT_ADDON(617,IAERA2(3,2));
	INSERT_ADDON(616,IAERA2(3,2));
	INSERT_ADDON(618,IAERA2(3,2));
	INSERT_ADDON(619,IAERA2(3,2));
	INSERT_ADDON(620,IAERA2(3,2));
	             
	INSERT_ADDON(621,IAERA2(4,1));
	INSERT_ADDON(622,IAERA2(4,1));
	INSERT_ADDON(623,IAERA2(4,1));
	INSERT_ADDON(624,IAERA2(4,1));
	INSERT_ADDON(625,IAERA2(4,1));

	INSERT_ADDON(626,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(627,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(628,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(629,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>);
	INSERT_ADDON(630,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>);

	INSERT_ADDON(631,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(632,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(633,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(634,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>);
	INSERT_ADDON(635,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>);

	INSERT_ADDON(636, enhance_speed_addon_point);
	INSERT_ADDON(637, enhance_speed_addon_point);
	INSERT_ADDON(638, enhance_speed_addon_point);
	INSERT_ADDON(639, enhance_speed_addon_point);
	INSERT_ADDON(640, enhance_speed_addon_point);

	INSERT_ADDON(641, enhance_armor_range_addon);
	INSERT_ADDON(642, enhance_armor_range_addon);
	INSERT_ADDON(643, enhance_armor_range_addon);
	INSERT_ADDON(644, enhance_armor_range_addon);
	INSERT_ADDON(645, enhance_armor_range_addon);

	INSERT_ADDON(586, IA_ED_ESS<offsetof(decoration_essence,damage)>); 
	INSERT_ADDON(587, item_decoration_specific_damage_addon);
	INSERT_ADDON(588, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>); 
	INSERT_ADDON(589, item_decoration_specific_magic_damage_addon);

	INSERT_ADDON(594, reduce_cast_time_addon);
	INSERT_ADDON(595, reduce_cast_time_addon);
	
	INSERT_ADDON(585, enhance_crit_rate);
	INSERT_ADDON(591, enhance_crit_rate);

	INSERT_ADDON(593, IA_ED_ESS<offsetof(decoration_essence,armor)>); 
	INSERT_ADDON(596, IA_ED_ESS<offsetof(decoration_essence,armor)>); 
	INSERT_ADDON(597, IA_ED_ESS<offsetof(decoration_essence,armor)>); 

//	INSERT_ADDON(512,enhance_str_addon2);

	INSERT_ADDON(652 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(653 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(654 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(655 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(656 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(657 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(658 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(659 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(660 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(661 ,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(1058,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(1057,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(1056,IA_EA_ESS<offsetof(armor_essence,defense)>);
	INSERT_ADDON(1055,IA_EA_ESS<offsetof(armor_essence,defense)>);


	INSERT_ADDON(662 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(663 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(664 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(665 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(666 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(667 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(668 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(669 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(670 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(671 ,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(1062,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(1061,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(1060,IA_EA_ESS<offsetof(armor_essence,armor)>);
	INSERT_ADDON(1059,IA_EA_ESS<offsetof(armor_essence,armor)>);

#define IAERA3(x,y) item_armor_enhance_resistance_addon_3<x,y>
	INSERT_ADDON(676 ,IAERA3(0,3));
	INSERT_ADDON(677 ,IAERA3(0,3));
	INSERT_ADDON(678 ,IAERA3(0,3));
	INSERT_ADDON(679 ,IAERA3(0,3));
	INSERT_ADDON(680 ,IAERA3(0,3));
	INSERT_ADDON(681 ,IAERA3(0,3));
	INSERT_ADDON(682 ,IAERA3(0,3));
	INSERT_ADDON(683 ,IAERA3(0,3));
	INSERT_ADDON(684 ,IAERA3(0,3));
	INSERT_ADDON(685 ,IAERA3(0,3));
	INSERT_ADDON(686 ,IAERA3(0,3));
	INSERT_ADDON(1066,IAERA3(0,3));
	INSERT_ADDON(1065,IAERA3(0,3));
	INSERT_ADDON(1064,IAERA3(0,3));
	INSERT_ADDON(1063,IAERA3(0,3));

	INSERT_ADDON(693 ,IAERA3(1,0));
	INSERT_ADDON(694 ,IAERA3(1,0));
	INSERT_ADDON(695 ,IAERA3(1,0));
	INSERT_ADDON(696 ,IAERA3(1,0));
	INSERT_ADDON(698 ,IAERA3(1,0));
	INSERT_ADDON(699 ,IAERA3(1,0));
	INSERT_ADDON(700 ,IAERA3(1,0));
	INSERT_ADDON(701 ,IAERA3(1,0));
	INSERT_ADDON(707 ,IAERA3(1,0));
	INSERT_ADDON(708 ,IAERA3(1,0));
	INSERT_ADDON(709 ,IAERA3(1,0));
	INSERT_ADDON(1070,IAERA3(1,0));
	INSERT_ADDON(1069,IAERA3(1,0));
	INSERT_ADDON(1068,IAERA3(1,0));
	INSERT_ADDON(1067,IAERA3(1,0));

	INSERT_ADDON(710 ,IAERA3(2,4));
	INSERT_ADDON(711 ,IAERA3(2,4));
	INSERT_ADDON(712 ,IAERA3(2,4));
	INSERT_ADDON(713 ,IAERA3(2,4));
	INSERT_ADDON(714 ,IAERA3(2,4));
	INSERT_ADDON(715 ,IAERA3(2,4));
	INSERT_ADDON(716 ,IAERA3(2,4));
	INSERT_ADDON(717 ,IAERA3(2,4));
	INSERT_ADDON(718 ,IAERA3(2,4));
	INSERT_ADDON(719 ,IAERA3(2,4));
	INSERT_ADDON(720 ,IAERA3(2,4));
	INSERT_ADDON(1074,IAERA3(2,4));
	INSERT_ADDON(1073,IAERA3(2,4));
	INSERT_ADDON(1072,IAERA3(2,4));
	INSERT_ADDON(1071,IAERA3(2,4));

	INSERT_ADDON(722 ,IAERA3(3,2));
	INSERT_ADDON(723 ,IAERA3(3,2));
	INSERT_ADDON(724 ,IAERA3(3,2));
	INSERT_ADDON(725 ,IAERA3(3,2));
	INSERT_ADDON(726 ,IAERA3(3,2));
	INSERT_ADDON(727 ,IAERA3(3,2));
	INSERT_ADDON(728 ,IAERA3(3,2));
	INSERT_ADDON(729 ,IAERA3(3,2));
	INSERT_ADDON(730 ,IAERA3(3,2));
	INSERT_ADDON(731 ,IAERA3(3,2));
	INSERT_ADDON(732 ,IAERA3(3,2));
	INSERT_ADDON(1078,IAERA3(3,2));
	INSERT_ADDON(1077,IAERA3(3,2));
	INSERT_ADDON(1076,IAERA3(3,2));
	INSERT_ADDON(1075,IAERA3(3,2));

	INSERT_ADDON(733 ,IAERA3(4,1));
	INSERT_ADDON(734 ,IAERA3(4,1));
	INSERT_ADDON(735 ,IAERA3(4,1));
	INSERT_ADDON(736 ,IAERA3(4,1));
	INSERT_ADDON(737 ,IAERA3(4,1));
	INSERT_ADDON(738 ,IAERA3(4,1));
	INSERT_ADDON(739 ,IAERA3(4,1));
	INSERT_ADDON(741 ,IAERA3(4,1));
	INSERT_ADDON(742 ,IAERA3(4,1));
	INSERT_ADDON(743 ,IAERA3(4,1));
	INSERT_ADDON(744 ,IAERA3(4,1));
	INSERT_ADDON(1082,IAERA3(4,1));
	INSERT_ADDON(1081,IAERA3(4,1));
	INSERT_ADDON(1080,IAERA3(4,1));
	INSERT_ADDON(1079,IAERA3(4,1));

	INSERT_ADDON(1124, enhance_str_addon);
	INSERT_ADDON(1125, enhance_str_addon);
	INSERT_ADDON(1126, enhance_str_addon);
	INSERT_ADDON(1127, enhance_str_addon);
	INSERT_ADDON(1128, enhance_str_addon);
	INSERT_ADDON(1104, enhance_str_addon);
	INSERT_ADDON(1105, enhance_str_addon);
	INSERT_ADDON(1106, enhance_str_addon);
	INSERT_ADDON(1107, enhance_str_addon);
	INSERT_ADDON(1108, enhance_str_addon);
	INSERT_ADDON(1098, enhance_str_addon);
	INSERT_ADDON(1084, enhance_str_addon);
	INSERT_ADDON(1085, enhance_str_addon);
	INSERT_ADDON(1086, enhance_str_addon);
	INSERT_ADDON(1087, enhance_str_addon);

	INSERT_ADDON(1088, enhance_agi_addon);
	INSERT_ADDON(1089, enhance_agi_addon);
	INSERT_ADDON(1090, enhance_agi_addon);
	INSERT_ADDON(1091, enhance_agi_addon);
	INSERT_ADDON(1092, enhance_agi_addon);
	INSERT_ADDON(1129, enhance_agi_addon);
	INSERT_ADDON(1130, enhance_agi_addon);
	INSERT_ADDON(1131, enhance_agi_addon);
	INSERT_ADDON(1132, enhance_agi_addon);
	INSERT_ADDON(1133, enhance_agi_addon);
	INSERT_ADDON(1109, enhance_agi_addon);
	INSERT_ADDON(1110, enhance_agi_addon);
	INSERT_ADDON(1111, enhance_agi_addon);
	INSERT_ADDON(1112, enhance_agi_addon);
	INSERT_ADDON(1113, enhance_agi_addon);

	INSERT_ADDON(1114, enhance_eng_addon);
	INSERT_ADDON(1115, enhance_eng_addon);
	INSERT_ADDON(1116, enhance_eng_addon);
	INSERT_ADDON(1117, enhance_eng_addon);
	INSERT_ADDON(1118, enhance_eng_addon);
	INSERT_ADDON(1134, enhance_eng_addon);
	INSERT_ADDON(1135, enhance_eng_addon);
	INSERT_ADDON(1136, enhance_eng_addon);
	INSERT_ADDON(1137, enhance_eng_addon);
	INSERT_ADDON(1138, enhance_eng_addon);
	INSERT_ADDON(1093, enhance_eng_addon);
	INSERT_ADDON(1094, enhance_eng_addon);
	INSERT_ADDON(1095, enhance_eng_addon);
	INSERT_ADDON(1096, enhance_eng_addon);
	INSERT_ADDON(1097, enhance_eng_addon);

	INSERT_ADDON(1099, enhance_vit_addon);
	INSERT_ADDON(1100, enhance_vit_addon);
	INSERT_ADDON(1101, enhance_vit_addon);
	INSERT_ADDON(1102, enhance_vit_addon);
	INSERT_ADDON(1103, enhance_vit_addon);
	INSERT_ADDON(1139, enhance_vit_addon);
	INSERT_ADDON(1140, enhance_vit_addon);
	INSERT_ADDON(1141, enhance_vit_addon);
	INSERT_ADDON(1142, enhance_vit_addon);
	INSERT_ADDON(1143, enhance_vit_addon);
	INSERT_ADDON(1119, enhance_vit_addon);
	INSERT_ADDON(1120, enhance_vit_addon);
	INSERT_ADDON(1121, enhance_vit_addon);
	INSERT_ADDON(1122, enhance_vit_addon);
	INSERT_ADDON(1123, enhance_vit_addon);

	INSERT_ADDON(753 , enhance_weapon_damage_addon);
	INSERT_ADDON(754 , enhance_weapon_damage_addon);
	INSERT_ADDON(756 , enhance_weapon_damage_addon);
	INSERT_ADDON(755 , enhance_weapon_damage_addon);
	INSERT_ADDON(757 , enhance_weapon_damage_addon);
	INSERT_ADDON(758 , enhance_weapon_damage_addon);
	INSERT_ADDON(759 , enhance_weapon_damage_addon);
	INSERT_ADDON(760 , enhance_weapon_damage_addon);
	INSERT_ADDON(761 , enhance_weapon_damage_addon);
	INSERT_ADDON(762 , enhance_weapon_damage_addon);
	INSERT_ADDON(763 , enhance_weapon_damage_addon);
	INSERT_ADDON(784 , enhance_weapon_damage_addon);
	INSERT_ADDON(785 , enhance_weapon_damage_addon);
	INSERT_ADDON(786 , enhance_weapon_damage_addon);
	INSERT_ADDON(787 , enhance_weapon_damage_addon);
	INSERT_ADDON(788 , enhance_weapon_damage_addon);
	INSERT_ADDON(789 , enhance_weapon_damage_addon);
	INSERT_ADDON(790 , enhance_weapon_damage_addon);
	INSERT_ADDON(791 , enhance_weapon_damage_addon);
	INSERT_ADDON(792 , enhance_weapon_damage_addon);
	INSERT_ADDON(793 , enhance_weapon_damage_addon);
	INSERT_ADDON(794 , enhance_weapon_damage_addon);
	INSERT_ADDON(994 , enhance_weapon_damage_addon);
	INSERT_ADDON(995 , enhance_weapon_damage_addon);
	INSERT_ADDON(996 , enhance_weapon_damage_addon);
	INSERT_ADDON(997 , enhance_weapon_damage_addon);
	INSERT_ADDON(1009, enhance_weapon_damage_addon);
	INSERT_ADDON(1008, enhance_weapon_damage_addon);
	INSERT_ADDON(1007, enhance_weapon_damage_addon);
	INSERT_ADDON(1006, enhance_weapon_damage_addon);

	INSERT_ADDON(795 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(796 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(797 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(798 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(799 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(800 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(801 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(802 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(803 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(804 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(805 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(1001, enhance_weapon_max_damage_addon);
	INSERT_ADDON(1000, enhance_weapon_max_damage_addon);
	INSERT_ADDON(999 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(998 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(764 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(765 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(766 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(767 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(768 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(769 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(770 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(771 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(772 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(773 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(774 , enhance_weapon_max_damage_addon);
	INSERT_ADDON(1003, enhance_weapon_max_damage_addon);
	INSERT_ADDON(1004, enhance_weapon_max_damage_addon);
	INSERT_ADDON(1005, enhance_weapon_max_damage_addon);
	INSERT_ADDON(1002, enhance_weapon_max_damage_addon);

typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,damage),DOUBLE_PERCENT> scale_enhance_damage2;
	INSERT_ADDON(775, scale_enhance_damage2);

	INSERT_ADDON(809, enhance_weapon_magic_addon);
	INSERT_ADDON(810, enhance_weapon_magic_addon);
	INSERT_ADDON(811, enhance_weapon_magic_addon);
	INSERT_ADDON(813, enhance_weapon_magic_addon);
	INSERT_ADDON(814, enhance_weapon_magic_addon);
	INSERT_ADDON(815, enhance_weapon_magic_addon);
	INSERT_ADDON(816, enhance_weapon_magic_addon);
	INSERT_ADDON(817, enhance_weapon_magic_addon);
	INSERT_ADDON(818, enhance_weapon_magic_addon);
	INSERT_ADDON(820, enhance_weapon_magic_addon);
	INSERT_ADDON(821, enhance_weapon_magic_addon);
	INSERT_ADDON(990, enhance_weapon_magic_addon);
	INSERT_ADDON(991, enhance_weapon_magic_addon);
	INSERT_ADDON(992, enhance_weapon_magic_addon);
	INSERT_ADDON(993, enhance_weapon_magic_addon);

	INSERT_ADDON(822, enhance_weapon_max_magic_addon);
	INSERT_ADDON(823, enhance_weapon_max_magic_addon);
	INSERT_ADDON(824, enhance_weapon_max_magic_addon);
	INSERT_ADDON(825, enhance_weapon_max_magic_addon);
	INSERT_ADDON(826, enhance_weapon_max_magic_addon);
	INSERT_ADDON(827, enhance_weapon_max_magic_addon);
	INSERT_ADDON(828, enhance_weapon_max_magic_addon);
	INSERT_ADDON(829, enhance_weapon_max_magic_addon);
	INSERT_ADDON(830, enhance_weapon_max_magic_addon);
	INSERT_ADDON(831, enhance_weapon_max_magic_addon);
	INSERT_ADDON(832, enhance_weapon_max_magic_addon);
	INSERT_ADDON(989, enhance_weapon_max_magic_addon);
	INSERT_ADDON(988, enhance_weapon_max_magic_addon);
	INSERT_ADDON(987, enhance_weapon_max_magic_addon);
	INSERT_ADDON(986, enhance_weapon_max_magic_addon);

typedef EPSA_EQ_addon<EQ_PERCENT+offsetof(equip_item::scale_data,magic_damage),DOUBLE_PERCENT> scale_enhance_magic_damage2;
	INSERT_ADDON(835, scale_enhance_magic_damage2);

	INSERT_ADDON(848 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(849 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(850 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(852 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(857 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(858 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(859 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(860 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(855 , IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(1011, IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(1010, IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(1012, IA_ED_ESS<offsetof(decoration_essence,damage)>);
	INSERT_ADDON(1013, IA_ED_ESS<offsetof(decoration_essence,damage)>);

	INSERT_ADDON(885 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(886 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(887 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(889 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(890 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(891 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(892 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(893 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(895 , IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(1018, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(1019, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(1020, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);
	INSERT_ADDON(1022, IA_ED_ESS<offsetof(decoration_essence,magic_damage)>);

	INSERT_ADDON(862  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(863  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(864  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(865  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(867  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(868  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(869  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(870  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(871  ,item_decoration_specific_damage_addon);
	INSERT_ADDON(1014 ,item_decoration_specific_damage_addon);
	INSERT_ADDON(1015 ,item_decoration_specific_damage_addon);
	INSERT_ADDON(1016 ,item_decoration_specific_damage_addon);
	INSERT_ADDON(1017 ,item_decoration_specific_damage_addon);

	INSERT_ADDON(896 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(897 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(898 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(899 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(900 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(901 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(902 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(903 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(905 ,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(1026,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(1025,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(1024,item_decoration_specific_magic_damage_addon);
	INSERT_ADDON(1023,item_decoration_specific_magic_damage_addon);

	INSERT_ADDON(909 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(910 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(911 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(912 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(913 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(914 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(915 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(916 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(917 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(918 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(919 , IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(1027, IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(1028, IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(1029, IA_ED_ESS<offsetof(decoration_essence,defense)>);
	INSERT_ADDON(1030, IA_ED_ESS<offsetof(decoration_essence,defense)>);

	INSERT_ADDON(920 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(921 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(922 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(923 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(924 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(925 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(926 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(927 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(928 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(929 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(930 , IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(1031, IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(1032, IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(1033, IA_ED_ESS<offsetof(decoration_essence,armor)>);
	INSERT_ADDON(1034, IA_ED_ESS<offsetof(decoration_essence,armor)>);

	INSERT_ADDON(931 ,IDMRA(0,3));
	INSERT_ADDON(932 ,IDMRA(0,3));
	INSERT_ADDON(933 ,IDMRA(0,3));
	INSERT_ADDON(934 ,IDMRA(0,3));
	INSERT_ADDON(935 ,IDMRA(0,3));
	INSERT_ADDON(936 ,IDMRA(0,3));
	INSERT_ADDON(937 ,IDMRA(0,3));
	INSERT_ADDON(938 ,IDMRA(0,3));
	INSERT_ADDON(939 ,IDMRA(0,3));
	INSERT_ADDON(940 ,IDMRA(0,3));
	INSERT_ADDON(941 ,IDMRA(0,3));
	INSERT_ADDON(1038,IDMRA(0,3));
	INSERT_ADDON(1037,IDMRA(0,3));
	INSERT_ADDON(1036,IDMRA(0,3));
	INSERT_ADDON(1035,IDMRA(0,3));

	INSERT_ADDON(942 ,IDMRA(1,0));
	INSERT_ADDON(943 ,IDMRA(1,0));
	INSERT_ADDON(944 ,IDMRA(1,0));
	INSERT_ADDON(945 ,IDMRA(1,0));
	INSERT_ADDON(946 ,IDMRA(1,0));
	INSERT_ADDON(947 ,IDMRA(1,0));
	INSERT_ADDON(948 ,IDMRA(1,0));
	INSERT_ADDON(949 ,IDMRA(1,0));
	INSERT_ADDON(950 ,IDMRA(1,0));
	INSERT_ADDON(951 ,IDMRA(1,0));
	INSERT_ADDON(952 ,IDMRA(1,0));
	INSERT_ADDON(1042,IDMRA(1,0));
	INSERT_ADDON(1041,IDMRA(1,0));
	INSERT_ADDON(1040,IDMRA(1,0));
	INSERT_ADDON(1039,IDMRA(1,0));

	INSERT_ADDON(964 ,IDMRA(2,4));
	INSERT_ADDON(965 ,IDMRA(2,4));
	INSERT_ADDON(966 ,IDMRA(2,4));
	INSERT_ADDON(967 ,IDMRA(2,4));
	INSERT_ADDON(968 ,IDMRA(2,4));
	INSERT_ADDON(969 ,IDMRA(2,4));
	INSERT_ADDON(970 ,IDMRA(2,4));
	INSERT_ADDON(971 ,IDMRA(2,4));
	INSERT_ADDON(972 ,IDMRA(2,4));
	INSERT_ADDON(973 ,IDMRA(2,4));
	INSERT_ADDON(974 ,IDMRA(2,4));
	INSERT_ADDON(1050,IDMRA(2,4));
	INSERT_ADDON(1049,IDMRA(2,4));
	INSERT_ADDON(1048,IDMRA(2,4));
	INSERT_ADDON(1047,IDMRA(2,4));

	INSERT_ADDON(975 ,IDMRA(3,2));
	INSERT_ADDON(976 ,IDMRA(3,2));
	INSERT_ADDON(977 ,IDMRA(3,2));
	INSERT_ADDON(978 ,IDMRA(3,2));
	INSERT_ADDON(979 ,IDMRA(3,2));
	INSERT_ADDON(980 ,IDMRA(3,2));
	INSERT_ADDON(981 ,IDMRA(3,2));
	INSERT_ADDON(982 ,IDMRA(3,2));
	INSERT_ADDON(983 ,IDMRA(3,2));
	INSERT_ADDON(984 ,IDMRA(3,2));
	INSERT_ADDON(985 ,IDMRA(3,2));
	INSERT_ADDON(1054,IDMRA(3,2));
	INSERT_ADDON(1053,IDMRA(3,2));
	INSERT_ADDON(1052,IDMRA(3,2));
	INSERT_ADDON(1051,IDMRA(3,2));

	INSERT_ADDON(953 ,IDMRA(4,1));
	INSERT_ADDON(954 ,IDMRA(4,1));
	INSERT_ADDON(956 ,IDMRA(4,1));
	INSERT_ADDON(955 ,IDMRA(4,1));
	INSERT_ADDON(957 ,IDMRA(4,1));
	INSERT_ADDON(958 ,IDMRA(4,1));
	INSERT_ADDON(959 ,IDMRA(4,1));
	INSERT_ADDON(960 ,IDMRA(4,1));
	INSERT_ADDON(961 ,IDMRA(4,1));
	INSERT_ADDON(962 ,IDMRA(4,1));
	INSERT_ADDON(963 ,IDMRA(4,1));
	INSERT_ADDON(1046,IDMRA(4,1));
	INSERT_ADDON(1045,IDMRA(4,1));
	INSERT_ADDON(1044,IDMRA(4,1));
	INSERT_ADDON(1043,IDMRA(4,1));

#define  STONE_MAGIC_RES_ADDON(x) EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,resistance) + sizeof(int) * (x), POINT>
#define  STONE_MAGIC_DMG_ADDON(x) EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,addon_damage) + sizeof(int) * (x), POINT>
	
	INSERT_ADDON(672 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(673 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(674 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(675 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(687 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(688 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(689 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(690 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(691 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(692 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(778 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(779 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(776 ,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(777 ,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(1184,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(1185,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(1186,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(1187,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(1188,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(1189,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(1190,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(1191,STONE_MAGIC_RES_ADDON(0));
	INSERT_ADDON(1192,STONE_MAGIC_DMG_ADDON(0)); 
	INSERT_ADDON(1193,STONE_MAGIC_RES_ADDON(0));

	INSERT_ADDON(706 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(721 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(740 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(745 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(746 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(747 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(748 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(749 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(750 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(751 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(780 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(781 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(782 ,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(783 ,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(1194,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(1195,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(1196,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(1197,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(1198,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(1199,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(1200,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(1201,STONE_MAGIC_RES_ADDON(1));
	INSERT_ADDON(1202,STONE_MAGIC_DMG_ADDON(1)); 
	INSERT_ADDON(1203,STONE_MAGIC_RES_ADDON(1));

	INSERT_ADDON(812 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(808 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(819 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(833 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(834 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(836 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(837 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(838 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(839 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(840 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(841 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(842 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(843 ,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(844 ,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(1204,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(1205,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(1206,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(1207,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(1208,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(1209,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(1210,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(1211,STONE_MAGIC_RES_ADDON(2));
	INSERT_ADDON(1212,STONE_MAGIC_DMG_ADDON(2)); 
	INSERT_ADDON(1213,STONE_MAGIC_RES_ADDON(2));

	INSERT_ADDON(845 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(846 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(847 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(851 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(853 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(854 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(856 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(861 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(866 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(872 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(873 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(874 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(875 ,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(876 ,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(1214,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(1215,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(1216,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(1217,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(1218,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(1219,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(1220,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(1221,STONE_MAGIC_RES_ADDON(3));
	INSERT_ADDON(1222,STONE_MAGIC_DMG_ADDON(3)); 
	INSERT_ADDON(1223,STONE_MAGIC_RES_ADDON(3));

	INSERT_ADDON(877 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(878 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(879 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(880 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(881 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(882 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(883 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(884 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(888 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(894 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(904 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(906 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(907 ,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(908 ,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(1224,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(1225,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(1226,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(1227,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(1228,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(1229,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(1230,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(1231,STONE_MAGIC_RES_ADDON(4));
	INSERT_ADDON(1232,STONE_MAGIC_DMG_ADDON(4)); 
	INSERT_ADDON(1233,STONE_MAGIC_RES_ADDON(4));
	
	INSERT_ADDON(1234, enhance_mp_addon_2);
	INSERT_ADDON(1235, enhance_mp_addon_2);
	INSERT_ADDON(1236, enhance_mp_addon_2);
	INSERT_ADDON(1237, enhance_mp_addon_2);
	INSERT_ADDON(1238, enhance_mp_addon_2);

	INSERT_ADDON(1239, enhance_hp_addon_2);
	INSERT_ADDON(1240, enhance_hp_addon_2);
	INSERT_ADDON(1241, enhance_hp_addon_2);
	INSERT_ADDON(1242, enhance_hp_addon_2);
	INSERT_ADDON(1243, enhance_hp_addon_2);

	INSERT_ADDON(1244,enhance_defense_addon);
	INSERT_ADDON(1245,enhance_defense_addon);
	INSERT_ADDON(1246,enhance_defense_addon);
	INSERT_ADDON(1247,enhance_defense_addon);
	INSERT_ADDON(1248,enhance_defense_addon);
	INSERT_ADDON(1249,enhance_defense_addon);
	INSERT_ADDON(1250,enhance_defense_addon);
	INSERT_ADDON(1251,enhance_defense_addon);
	INSERT_ADDON(1252,enhance_defense_addon);
	INSERT_ADDON(1253,enhance_defense_addon);
	INSERT_ADDON(1254,enhance_defense_addon);
	INSERT_ADDON(1255,enhance_defense_addon);
	INSERT_ADDON(1256,enhance_defense_addon);
	INSERT_ADDON(1257,enhance_defense_addon);

	INSERT_ADDON(1258, enhance_armor_range_addon);
	INSERT_ADDON(1259, enhance_armor_range_addon);
	INSERT_ADDON(1260, enhance_armor_range_addon);
	INSERT_ADDON(1261, enhance_armor_range_addon);
	INSERT_ADDON(1262, enhance_armor_range_addon);
	INSERT_ADDON(1263, enhance_armor_range_addon);
	INSERT_ADDON(1264, enhance_armor_range_addon);
	INSERT_ADDON(1265, enhance_armor_range_addon);
	INSERT_ADDON(1266, enhance_armor_range_addon);
	INSERT_ADDON(1267, enhance_armor_range_addon);
	INSERT_ADDON(1268, enhance_armor_range_addon);
	INSERT_ADDON(1269, enhance_armor_range_addon);
	INSERT_ADDON(1270, enhance_armor_range_addon);
	INSERT_ADDON(1271, enhance_armor_range_addon);
	
	INSERT_ADDON(1272, enhance_damage_reduce_addon);
	INSERT_ADDON(1273, enhance_damage_reduce_addon);
	INSERT_ADDON(1274, enhance_damage_reduce_addon);

	INSERT_ADDON(1275, item_skill_addon);
	INSERT_ADDON(1276, item_skill_addon);
	INSERT_ADDON(1277, item_skill_addon);
	INSERT_ADDON(1278, item_skill_addon);
	INSERT_ADDON(1279, item_skill_addon);
	INSERT_ADDON(1280, item_skill_addon);
	INSERT_ADDON(1281, item_skill_addon);
	INSERT_ADDON(1282, item_skill_addon);
	INSERT_ADDON(1283, item_skill_addon);
	INSERT_ADDON(1284, item_skill_addon);
	INSERT_ADDON(1285, item_skill_addon);
	INSERT_ADDON(1286, item_skill_addon);
	INSERT_ADDON(1287, item_skill_addon);
	INSERT_ADDON(1288, item_skill_addon);
	INSERT_ADDON(1289, item_skill_addon);
	INSERT_ADDON(1290, item_skill_addon);
	INSERT_ADDON(1291, item_skill_addon);
	INSERT_ADDON(1292, item_skill_addon);
	INSERT_ADDON(1293, item_skill_addon);
	INSERT_ADDON(1294, item_skill_addon);
	INSERT_ADDON(1295, item_skill_addon);
	INSERT_ADDON(1296, item_skill_addon);
	INSERT_ADDON(1297, item_skill_addon);
	INSERT_ADDON(1298, item_skill_addon);
	INSERT_ADDON(1299, item_skill_addon);
	INSERT_ADDON(1300, item_skill_addon);
	INSERT_ADDON(1301, item_skill_addon);
	INSERT_ADDON(1302, item_skill_addon);
	INSERT_ADDON(1303, item_skill_addon);
	INSERT_ADDON(1304, item_skill_addon);
	INSERT_ADDON(1305, item_skill_addon);
	INSERT_ADDON(1306, item_skill_addon);
	INSERT_ADDON(1307, item_skill_addon);
	INSERT_ADDON(1308, item_skill_addon);
	INSERT_ADDON(1309, item_skill_addon);
	INSERT_ADDON(1310, item_skill_addon);

	INSERT_ADDON(1311, enhance_attack_addon_2);
	INSERT_ADDON(1312, enhance_attack_addon_2);
	INSERT_ADDON(1313, enhance_attack_addon_2);
	INSERT_ADDON(1314, enhance_attack_addon_2);
	INSERT_ADDON(1315, enhance_attack_addon_2);
	INSERT_ADDON(1316, enhance_attack_addon_2);
	INSERT_ADDON(1317, enhance_attack_addon_2);
	INSERT_ADDON(1318, enhance_attack_addon_2);
	INSERT_ADDON(1319, enhance_attack_addon_2);
	INSERT_ADDON(1320, enhance_attack_addon_2);
	INSERT_ADDON(1321, enhance_attack_addon_2);
	INSERT_ADDON(1322, enhance_attack_addon_2);
	INSERT_ADDON(1323, enhance_attack_addon_2);
	INSERT_ADDON(1324, enhance_attack_addon_2);

	INSERT_ADDON(1325, enhance_mp_addon_2);
	INSERT_ADDON(1326, enhance_mp_addon_2);
	INSERT_ADDON(1327, enhance_mp_addon_2);
	INSERT_ADDON(1328, enhance_mp_addon_2);
	INSERT_ADDON(1329, enhance_mp_addon_2);
	INSERT_ADDON(1330, enhance_mp_addon_2);
	INSERT_ADDON(1331, enhance_mp_addon_2);
	INSERT_ADDON(1332, enhance_mp_addon_2);
	INSERT_ADDON(1333, enhance_mp_addon_2);
	INSERT_ADDON(1334, enhance_mp_addon_2);

	INSERT_ADDON(1335, enhance_hp_addon_2);
	INSERT_ADDON(1336, enhance_hp_addon_2);
	INSERT_ADDON(1337, enhance_hp_addon_2);
	INSERT_ADDON(1338, enhance_hp_addon_2);
	INSERT_ADDON(1339, enhance_hp_addon_2);
	INSERT_ADDON(1340, enhance_hp_addon_2);
	INSERT_ADDON(1341, enhance_hp_addon_2);
	INSERT_ADDON(1342, enhance_hp_addon_2);
	INSERT_ADDON(1343, enhance_hp_addon_2);
	INSERT_ADDON(1344, enhance_hp_addon_2);
	
	INSERT_ADDON(1401, enhance_weapon_damage_addon);
	INSERT_ADDON(1402, enhance_weapon_damage_addon);
	INSERT_ADDON(1403, enhance_weapon_damage_addon);
	INSERT_ADDON(1404, enhance_weapon_damage_addon);
	INSERT_ADDON(1405, enhance_weapon_damage_addon);

	INSERT_ADDON(1406, enhance_attack_addon_2);
	INSERT_ADDON(1407, enhance_attack_addon_2);
	INSERT_ADDON(1408, enhance_attack_addon_2);

	INSERT_ADDON(1409, enhance_armor_range_addon);
	INSERT_ADDON(1410, enhance_armor_range_addon);
	
	INSERT_ADDON(1411, enhance_hp_addon_2);
	INSERT_ADDON(1412, enhance_hp_addon_2);
	INSERT_ADDON(1413, enhance_hp_addon_2);
	INSERT_ADDON(1414, enhance_hp_addon_2);
	INSERT_ADDON(1415, enhance_hp_addon_2);
	INSERT_ADDON(1416, enhance_hp_addon_2);

	INSERT_ADDON(1417, reduce_cast_time_addon);
	
	INSERT_ADDON(1418, enhance_agi_addon);
	
	INSERT_ADDON(1419, enhance_eng_addon);
	
	INSERT_ADDON(1420, enhance_defense_addon);

	INSERT_ADDON(1421, enhance_crit_rate);

	INSERT_ADDON(1422, enhance_weapon_attack_range);

	INSERT_ADDON(1345,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(1346,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(1347,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(1348,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 
	INSERT_ADDON(1349,IA_EA_ESS<offsetof(armor_essence,hp_enhance)>); 

	INSERT_ADDON(1350,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(1351,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(1352,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(1353,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 
	INSERT_ADDON(1354,IA_EA_ESS<offsetof(armor_essence,mp_enhance)>); 

	INSERT_ADDON(1355, enhance_mp_addon_2);
	INSERT_ADDON(1356, enhance_mp_addon_2);
	INSERT_ADDON(1357, enhance_mp_addon_2);
	INSERT_ADDON(1358, enhance_mp_addon_2);
	INSERT_ADDON(1359, enhance_mp_addon_2);
	INSERT_ADDON(1360, enhance_mp_addon_2);
	INSERT_ADDON(1361, enhance_mp_addon_2);
	INSERT_ADDON(1362, enhance_mp_addon_2);
	INSERT_ADDON(1363, enhance_mp_addon_2);
	INSERT_ADDON(1364, enhance_mp_addon_2);
	INSERT_ADDON(1365, enhance_mp_addon_2);
	INSERT_ADDON(1366, enhance_mp_addon_2);
	INSERT_ADDON(1367, enhance_mp_addon_2);
	INSERT_ADDON(1368, enhance_mp_addon_2);
	INSERT_ADDON(1369, enhance_mp_addon_2);
	             
	INSERT_ADDON(1370, enhance_hp_addon_2);
	INSERT_ADDON(1371, enhance_hp_addon_2);
	INSERT_ADDON(1372, enhance_hp_addon_2);
	INSERT_ADDON(1373, enhance_hp_addon_2);
	INSERT_ADDON(1374, enhance_hp_addon_2);
	INSERT_ADDON(1375, enhance_hp_addon_2);
	INSERT_ADDON(1376, enhance_hp_addon_2);
	INSERT_ADDON(1377, enhance_hp_addon_2);
	INSERT_ADDON(1378, enhance_hp_addon_2);
	INSERT_ADDON(1379, enhance_hp_addon_2);
	INSERT_ADDON(1380, enhance_hp_addon_2);
	INSERT_ADDON(1381, enhance_hp_addon_2);
	INSERT_ADDON(1382, enhance_hp_addon_2);
	INSERT_ADDON(1383, enhance_hp_addon_2);
	INSERT_ADDON(1384, enhance_hp_addon_2);
	
	INSERT_ADDON(1385, enhance_attack_addon_2);
	INSERT_ADDON(1386, enhance_attack_addon_2);
	INSERT_ADDON(1387, enhance_attack_addon_2);
	INSERT_ADDON(1388, enhance_attack_addon_2);
	INSERT_ADDON(1389, enhance_attack_addon_2);
	INSERT_ADDON(1390, enhance_attack_addon_2);
	INSERT_ADDON(1391, enhance_attack_addon_2);
	INSERT_ADDON(1392, enhance_attack_addon_2);
	INSERT_ADDON(1393, enhance_attack_addon_2);
	INSERT_ADDON(1395, enhance_attack_addon_2);
	INSERT_ADDON(1396, enhance_attack_addon_2);
	INSERT_ADDON(1397, enhance_attack_addon_2);
	INSERT_ADDON(1398, enhance_attack_addon_2);
	INSERT_ADDON(1399, enhance_attack_addon_2);

	INSERT_ADDON(1423, enhance_weapon_magic_addon);
	INSERT_ADDON(1424, enhance_weapon_magic_addon);
	INSERT_ADDON(1425, enhance_weapon_magic_addon);
	INSERT_ADDON(1426, enhance_weapon_magic_addon);
	INSERT_ADDON(1427, enhance_weapon_magic_addon);

	INSERT_ADDON(1429, enhance_str_addon);
	INSERT_ADDON(1430, enhance_str_addon);
	INSERT_ADDON(1431, enhance_str_addon);
	INSERT_ADDON(1432, enhance_str_addon);
	INSERT_ADDON(1433, enhance_str_addon);
	INSERT_ADDON(1470, enhance_str_addon);
	INSERT_ADDON(1471, enhance_str_addon);
	INSERT_ADDON(1472, enhance_str_addon);
	INSERT_ADDON(1473, enhance_str_addon);
	INSERT_ADDON(1474, enhance_str_addon);
	INSERT_ADDON(1450, enhance_str_addon);
	INSERT_ADDON(1451, enhance_str_addon);
	INSERT_ADDON(1452, enhance_str_addon);
	INSERT_ADDON(1453, enhance_str_addon);
	INSERT_ADDON(1454, enhance_str_addon);

	INSERT_ADDON(1434, enhance_agi_addon);
	INSERT_ADDON(1435, enhance_agi_addon);
	INSERT_ADDON(1436, enhance_agi_addon);
	INSERT_ADDON(1437, enhance_agi_addon);
	INSERT_ADDON(1438, enhance_agi_addon);
	INSERT_ADDON(1475, enhance_agi_addon);
	INSERT_ADDON(1476, enhance_agi_addon);
	INSERT_ADDON(1477, enhance_agi_addon);
	INSERT_ADDON(1478, enhance_agi_addon);
	INSERT_ADDON(1479, enhance_agi_addon);
	INSERT_ADDON(1455, enhance_agi_addon);
	INSERT_ADDON(1456, enhance_agi_addon);
	INSERT_ADDON(1457, enhance_agi_addon);
	INSERT_ADDON(1458, enhance_agi_addon);
	INSERT_ADDON(1459, enhance_agi_addon);
	
	INSERT_ADDON(1444, enhance_vit_addon);
	INSERT_ADDON(1445, enhance_vit_addon);
	INSERT_ADDON(1446, enhance_vit_addon);
	INSERT_ADDON(1447, enhance_vit_addon);
	INSERT_ADDON(1448, enhance_vit_addon);
	INSERT_ADDON(1485, enhance_vit_addon);
	INSERT_ADDON(1486, enhance_vit_addon);
	INSERT_ADDON(1487, enhance_vit_addon);
	INSERT_ADDON(1488, enhance_vit_addon);
	INSERT_ADDON(1489, enhance_vit_addon);
	INSERT_ADDON(1465, enhance_vit_addon);
	INSERT_ADDON(1466, enhance_vit_addon);
	INSERT_ADDON(1467, enhance_vit_addon);
	INSERT_ADDON(1468, enhance_vit_addon);
	INSERT_ADDON(1469, enhance_vit_addon);

	INSERT_ADDON(1460, enhance_eng_addon);
	INSERT_ADDON(1461, enhance_eng_addon);
	INSERT_ADDON(1462, enhance_eng_addon);
	INSERT_ADDON(1463, enhance_eng_addon);
	INSERT_ADDON(1464, enhance_eng_addon);
	INSERT_ADDON(1480, enhance_eng_addon);
	INSERT_ADDON(1481, enhance_eng_addon);
	INSERT_ADDON(1482, enhance_eng_addon);
	INSERT_ADDON(1483, enhance_eng_addon);
	INSERT_ADDON(1484, enhance_eng_addon);
	INSERT_ADDON(1439, enhance_eng_addon);
	INSERT_ADDON(1440, enhance_eng_addon);
	INSERT_ADDON(1441, enhance_eng_addon);
	INSERT_ADDON(1442, enhance_eng_addon);
	INSERT_ADDON(1443, enhance_eng_addon);
	
	INSERT_ADDON(1428, reduce_cast_time_addon);
	INSERT_ADDON(1449, reduce_cast_time_addon);

	INSERT_ADDON(1490, enhance_str_addon);
	INSERT_ADDON(1491, enhance_vit_addon);

	INSERT_ADDON(1492, enhance_str_addon);
	INSERT_ADDON(1493, enhance_vit_addon);
	INSERT_ADDON(1494, enhance_agi_addon);
	INSERT_ADDON(1495, enhance_eng_addon);

	INSERT_ADDON(1496, refine_magic_damage);
	INSERT_ADDON(1497, refine_damage);
	INSERT_ADDON(1498, refine_damage);
	INSERT_ADDON(1499, refine_damage);

	INSERT_ADDON(1500, refine_max_hp);
	INSERT_ADDON(1501, refine_max_hp);
	INSERT_ADDON(1502, refine_max_hp);

	INSERT_ADDON(1503, refine_defense);
	INSERT_ADDON(1504, refine_resistance);
	INSERT_ADDON(1505, refine_armor);

	INSERT_ADDON(1510, enhance_crit_rate);
	INSERT_ADDON(1511, reduce_cast_time_addon);
	INSERT_ADDON(1512, enhance_attack_scale_addon);

	INSERT_ADDON(1513, enhance_str_addon);
	INSERT_ADDON(1514, enhance_agi_addon);
	INSERT_ADDON(1515, enhance_vit_addon);
	INSERT_ADDON(1516, enhance_eng_addon);
	INSERT_ADDON(1517, enhance_crit_rate);
	INSERT_ADDON(1518, reduce_cast_time_addon);

	INSERT_ADDON(1519,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1520,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1521,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1524,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1527,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1528,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1530,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1531,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1534,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1535,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1537,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1539,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1541,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1542,item_armor_enhance_resistance<0>);
	INSERT_ADDON(1544,item_armor_enhance_resistance<0>);
		
	INSERT_ADDON(1546,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1549,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1550,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1552,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1553,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1555,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1556,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1558,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1559,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1561,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1562,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1564,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1566,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1567,item_armor_enhance_resistance<1>);
	INSERT_ADDON(1568,item_armor_enhance_resistance<1>);
	
	
	INSERT_ADDON(1570,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1572,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1573,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1575,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1577,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1578,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1580,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1581,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1583,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1585,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1586,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1587,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1589,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1591,item_armor_enhance_resistance<2>);
	INSERT_ADDON(1592,item_armor_enhance_resistance<2>);
	
	INSERT_ADDON(1594,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1596,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1597,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1599,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1601,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1602,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1604,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1606,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1607,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1609,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1612,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1613,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1614,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1617,item_armor_enhance_resistance<3>);
	INSERT_ADDON(1618,item_armor_enhance_resistance<3>);

	INSERT_ADDON(1620,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1622,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1624,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1625,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1627,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1629,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1631,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1632,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1638,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1640,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1642,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1643,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1645,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1647,item_armor_enhance_resistance<4>);
	INSERT_ADDON(1649,item_armor_enhance_resistance<4>);

	INSERT_ADDON(1523,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1525,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1526,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1529,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1532,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1533,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1536,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1538,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1540,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1545,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1547,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1687,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1551,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1554,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1557,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1560,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1563,item_decoration_enchance_resistance<0>);
	INSERT_ADDON(1565,item_decoration_enchance_resistance<0>);

	INSERT_ADDON(1569,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1571,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1574,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1576,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1579,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1582,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1584,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1588,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1590,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1593,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1595,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1598,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1600,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1603,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1605,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1608,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1610,item_decoration_enchance_resistance<1>);
	INSERT_ADDON(1611,item_decoration_enchance_resistance<1>);
		
	INSERT_ADDON(1616,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1619,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1621,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1623,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1626,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1628,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1630,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1633,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1634,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1635,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1636,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1637,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1639,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1641,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1644,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1646,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1648,item_decoration_enchance_resistance<2>);
	INSERT_ADDON(1650,item_decoration_enchance_resistance<2>);

	INSERT_ADDON(1651,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1652,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1653,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1654,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1655,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1656,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1657,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1658,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1659,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1660,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1661,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1662,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1663,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1664,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1665,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1666,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1667,item_decoration_enchance_resistance<3>);
	INSERT_ADDON(1668,item_decoration_enchance_resistance<3>);

	INSERT_ADDON(1669,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1670,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1671,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1672,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1673,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1674,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1675,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1676,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1677,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1678,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1679,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1680,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1681,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1682,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1683,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1684,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1685,item_decoration_enchance_resistance<4>);
	INSERT_ADDON(1686,item_decoration_enchance_resistance<4>);

	INSERT_ADDON(1688, enhance_hp_scale_addon);
	INSERT_ADDON(1689, enhance_mp_scale_addon);

	INSERT_ADDON(1690, enhance_hpgen_addon);
	INSERT_ADDON(1691, enhance_mpgen_addon);

	INSERT_ADDON(1692, refine_damage);
	INSERT_ADDON(1693, refine_damage);
	INSERT_ADDON(1694, refine_damage);
	INSERT_ADDON(1695, refine_damage);
	INSERT_ADDON(1696, refine_damage);
	INSERT_ADDON(1697, refine_damage);
	INSERT_ADDON(1698, refine_damage);
	INSERT_ADDON(1701, refine_damage);
	INSERT_ADDON(1699, refine_damage);
	INSERT_ADDON(1700, refine_damage);
	INSERT_ADDON(1702, refine_damage);
	INSERT_ADDON(1703, refine_damage);
	INSERT_ADDON(1704, refine_damage);
	INSERT_ADDON(1705, refine_damage);
	INSERT_ADDON(1706, refine_damage);
	INSERT_ADDON(1707, refine_damage);
	INSERT_ADDON(1708, refine_damage);
	INSERT_ADDON(1709, refine_damage);
	INSERT_ADDON(1710, refine_damage);
	INSERT_ADDON(1711, refine_damage);
	INSERT_ADDON(1712, refine_damage);
	INSERT_ADDON(1713, refine_damage);
	INSERT_ADDON(1714, refine_damage);
	INSERT_ADDON(1715, refine_damage);
	INSERT_ADDON(1716, refine_damage);
	INSERT_ADDON(1717, refine_damage);
	INSERT_ADDON(1718, refine_damage);
	INSERT_ADDON(1719, refine_damage);
	INSERT_ADDON(1720, refine_damage);
	INSERT_ADDON(1721, refine_damage);
	INSERT_ADDON(1722, refine_damage);
	INSERT_ADDON(1723, refine_damage);
	INSERT_ADDON(1724, refine_damage);
	INSERT_ADDON(1725, refine_damage);
	INSERT_ADDON(1726, refine_damage);
	INSERT_ADDON(1727, refine_damage);
	INSERT_ADDON(1728, refine_damage);
	INSERT_ADDON(1729, refine_damage);
	INSERT_ADDON(1730, refine_damage);
	INSERT_ADDON(1731, refine_damage);
	INSERT_ADDON(1732, refine_damage);
	INSERT_ADDON(1733, refine_damage);
	INSERT_ADDON(1734, refine_damage);
	INSERT_ADDON(1735, refine_damage);
	INSERT_ADDON(1736, refine_damage);
	INSERT_ADDON(1737, refine_damage);
	INSERT_ADDON(1738, refine_damage);
	INSERT_ADDON(1739, refine_damage);
	INSERT_ADDON(1740, refine_damage);
	INSERT_ADDON(1741, refine_damage);
	INSERT_ADDON(1742, refine_damage);
	INSERT_ADDON(1743, refine_damage);
	INSERT_ADDON(1744, refine_damage);
	INSERT_ADDON(1745, refine_damage);
	INSERT_ADDON(1746, refine_damage);
	INSERT_ADDON(1747, refine_damage);
	INSERT_ADDON(1748, refine_damage);
	INSERT_ADDON(1749, refine_damage);
	INSERT_ADDON(1750, refine_damage);
	INSERT_ADDON(1751, refine_damage);

	INSERT_ADDON(1752, refine_magic_damage);
	INSERT_ADDON(1753, refine_magic_damage);
	INSERT_ADDON(1754, refine_magic_damage);
	INSERT_ADDON(1755, refine_magic_damage);
	INSERT_ADDON(1756, refine_magic_damage);
	INSERT_ADDON(1757, refine_magic_damage);
	INSERT_ADDON(1758, refine_magic_damage);
	INSERT_ADDON(1759, refine_magic_damage);
	INSERT_ADDON(1760, refine_magic_damage);
	INSERT_ADDON(1761, refine_magic_damage);
	INSERT_ADDON(1762, refine_magic_damage);
	INSERT_ADDON(1763, refine_magic_damage);
	INSERT_ADDON(1764, refine_magic_damage);
	INSERT_ADDON(1765, refine_magic_damage);
	INSERT_ADDON(1766, refine_magic_damage);
	INSERT_ADDON(1767, refine_magic_damage);
	INSERT_ADDON(1768, refine_magic_damage);
	INSERT_ADDON(1769, refine_magic_damage);
	INSERT_ADDON(1770, refine_magic_damage);
	INSERT_ADDON(1771, refine_magic_damage);
	
	INSERT_ADDON(1772, refine_max_hp);
	INSERT_ADDON(1773, refine_max_hp);
	INSERT_ADDON(1774, refine_max_hp);
	INSERT_ADDON(1775, refine_max_hp);
	INSERT_ADDON(1776, refine_max_hp);
	INSERT_ADDON(1777, refine_max_hp);
	INSERT_ADDON(1778, refine_max_hp);
	INSERT_ADDON(1779, refine_max_hp);
	INSERT_ADDON(1780, refine_max_hp);
	INSERT_ADDON(1781, refine_max_hp);
	INSERT_ADDON(1782, refine_max_hp);
	INSERT_ADDON(1783, refine_max_hp);
	INSERT_ADDON(1784, refine_max_hp);
	INSERT_ADDON(1785, refine_max_hp);
	INSERT_ADDON(1786, refine_max_hp);
	INSERT_ADDON(1787, refine_max_hp);
	INSERT_ADDON(1788, refine_max_hp);
	INSERT_ADDON(1789, refine_max_hp);
	INSERT_ADDON(1790, refine_max_hp);
	INSERT_ADDON(1791, refine_max_hp);
	INSERT_ADDON(1792, refine_max_hp);
	INSERT_ADDON(1793, refine_max_hp);
	INSERT_ADDON(1794, refine_max_hp);
	INSERT_ADDON(1795, refine_max_hp);
	INSERT_ADDON(1796, refine_max_hp);
	INSERT_ADDON(1797, refine_max_hp);
	INSERT_ADDON(1798, refine_max_hp);
	INSERT_ADDON(1799, refine_max_hp);
	INSERT_ADDON(1800, refine_max_hp);
	INSERT_ADDON(1801, refine_max_hp);
	INSERT_ADDON(1802, refine_max_hp);
	INSERT_ADDON(1803, refine_max_hp);
	INSERT_ADDON(1804, refine_max_hp);
	INSERT_ADDON(1805, refine_max_hp);
	INSERT_ADDON(1806, refine_max_hp);
	INSERT_ADDON(1807, refine_max_hp);
	INSERT_ADDON(1808, refine_max_hp);
	INSERT_ADDON(1809, refine_max_hp);
	INSERT_ADDON(1810, refine_max_hp);
	INSERT_ADDON(1811, refine_max_hp);
	INSERT_ADDON(1812, refine_max_hp);
	INSERT_ADDON(1813, refine_max_hp);
	INSERT_ADDON(1814, refine_max_hp);
	INSERT_ADDON(1815, refine_max_hp);
	INSERT_ADDON(1816, refine_max_hp);
	INSERT_ADDON(1817, refine_max_hp);
	INSERT_ADDON(1818, refine_max_hp);
	INSERT_ADDON(1819, refine_max_hp);
	INSERT_ADDON(1820, refine_max_hp);
	INSERT_ADDON(1821, refine_max_hp);
	INSERT_ADDON(1822, refine_max_hp);
	INSERT_ADDON(1823, refine_max_hp);
	INSERT_ADDON(1824, refine_max_hp);
	INSERT_ADDON(1825, refine_max_hp);
	INSERT_ADDON(1826, refine_max_hp);
	INSERT_ADDON(1827, refine_max_hp);
	INSERT_ADDON(1828, refine_max_hp);
	INSERT_ADDON(1829, refine_max_hp);
	INSERT_ADDON(1830, refine_max_hp);

	INSERT_ADDON(1832, refine_defense);
	INSERT_ADDON(1833, refine_defense);
	INSERT_ADDON(1834, refine_defense);
	INSERT_ADDON(1835, refine_defense);
	INSERT_ADDON(1836, refine_defense);
	INSERT_ADDON(1837, refine_defense);
	INSERT_ADDON(1838, refine_defense);
	INSERT_ADDON(1839, refine_defense);
	INSERT_ADDON(1840, refine_defense);
	INSERT_ADDON(1841, refine_defense);
	INSERT_ADDON(1842, refine_defense);
	INSERT_ADDON(1843, refine_defense);
	INSERT_ADDON(1844, refine_defense);
	INSERT_ADDON(1845, refine_defense);
	INSERT_ADDON(1846, refine_defense);
	INSERT_ADDON(1847, refine_defense);
	INSERT_ADDON(1848, refine_defense);
	INSERT_ADDON(1849, refine_defense);
	INSERT_ADDON(1850, refine_defense);
	INSERT_ADDON(1851, refine_defense);
	
	INSERT_ADDON(1852, refine_resistance);
	INSERT_ADDON(1853, refine_resistance);
	INSERT_ADDON(1854, refine_resistance);
	INSERT_ADDON(1855, refine_resistance);
	INSERT_ADDON(1856, refine_resistance);
	INSERT_ADDON(1857, refine_resistance);
	INSERT_ADDON(1858, refine_resistance);
	INSERT_ADDON(1859, refine_resistance);
	INSERT_ADDON(1860, refine_resistance);
	INSERT_ADDON(1861, refine_resistance);
	INSERT_ADDON(1862, refine_resistance);
	INSERT_ADDON(1863, refine_resistance);
	INSERT_ADDON(1864, refine_resistance);
	INSERT_ADDON(1865, refine_resistance);
	INSERT_ADDON(1866, refine_resistance);
	INSERT_ADDON(1867, refine_resistance);
	INSERT_ADDON(1868, refine_resistance);
	INSERT_ADDON(1869, refine_resistance);
	INSERT_ADDON(1870, refine_resistance);
	INSERT_ADDON(1871, refine_resistance);
	
	INSERT_ADDON(1872 , refine_armor);
	INSERT_ADDON(1873 , refine_armor);
	INSERT_ADDON(1874 , refine_armor);
	INSERT_ADDON(1875 , refine_armor);
	INSERT_ADDON(1876 , refine_armor);
	INSERT_ADDON(1877 , refine_armor);
	INSERT_ADDON(1878 , refine_armor);
	INSERT_ADDON(1879 , refine_armor);
	INSERT_ADDON(1880 , refine_armor);
	INSERT_ADDON(1881 , refine_armor);
	INSERT_ADDON(1882 , refine_armor);
	INSERT_ADDON(1883 , refine_armor);
	INSERT_ADDON(1884 , refine_armor);
	INSERT_ADDON(1885 , refine_armor);
	INSERT_ADDON(1886 , refine_armor);
	INSERT_ADDON(1887 , refine_armor);
	INSERT_ADDON(1888 , refine_armor);
	INSERT_ADDON(1889 , refine_armor);
	INSERT_ADDON(1890 , refine_armor);
	INSERT_ADDON(1891 , refine_armor);
	

	INSERT_ADDON(1912, enhance_str_addon);
	INSERT_ADDON(1913, enhance_str_addon);
	INSERT_ADDON(1914, enhance_str_addon);
	INSERT_ADDON(1915, enhance_str_addon);
	INSERT_ADDON(1916, enhance_str_addon);
	INSERT_ADDON(1917, enhance_str_addon);

	INSERT_ADDON(1918, enhance_agi_addon);
	INSERT_ADDON(1919, enhance_agi_addon);
	INSERT_ADDON(1920, enhance_agi_addon);
	INSERT_ADDON(1921, enhance_agi_addon);
	INSERT_ADDON(1922, enhance_agi_addon);
	INSERT_ADDON(1923, enhance_agi_addon);

	INSERT_ADDON(1924, enhance_vit_addon);
	INSERT_ADDON(1925, enhance_vit_addon);
	INSERT_ADDON(1926, enhance_vit_addon);
	INSERT_ADDON(1927, enhance_vit_addon);
	INSERT_ADDON(1928, enhance_vit_addon);
	INSERT_ADDON(1929, enhance_vit_addon);

	INSERT_ADDON(1930, enhance_eng_addon);
	INSERT_ADDON(1931, enhance_eng_addon);
	INSERT_ADDON(1932, enhance_eng_addon);
	INSERT_ADDON(1933, enhance_eng_addon);
	INSERT_ADDON(1934, enhance_eng_addon);
	INSERT_ADDON(1935, enhance_eng_addon);


	INSERT_ADDON(1936, enhance_str_addon);
	INSERT_ADDON(1937, enhance_str_addon);
	INSERT_ADDON(1938, enhance_str_addon);
	INSERT_ADDON(1939, enhance_str_addon);
	INSERT_ADDON(1940, enhance_str_addon);
	INSERT_ADDON(1941, enhance_str_addon);
	INSERT_ADDON(1942, enhance_str_addon);
	INSERT_ADDON(1943, enhance_str_addon);
	INSERT_ADDON(1944, enhance_str_addon);


	INSERT_ADDON(1945, enhance_agi_addon);
	INSERT_ADDON(1946, enhance_agi_addon);
	INSERT_ADDON(1947, enhance_agi_addon);
	INSERT_ADDON(1948, enhance_agi_addon);
	INSERT_ADDON(1949, enhance_agi_addon);
	INSERT_ADDON(1950, enhance_agi_addon);
	INSERT_ADDON(1951, enhance_agi_addon);
	INSERT_ADDON(1952, enhance_agi_addon);
	INSERT_ADDON(1953, enhance_agi_addon);
	
	
	INSERT_ADDON(1954, enhance_vit_addon);
	INSERT_ADDON(1955, enhance_vit_addon);
	INSERT_ADDON(1956, enhance_vit_addon);
	INSERT_ADDON(1957, enhance_vit_addon);
	INSERT_ADDON(1958, enhance_vit_addon);
	INSERT_ADDON(1959, enhance_vit_addon);
	INSERT_ADDON(1960, enhance_vit_addon);
	INSERT_ADDON(1961, enhance_vit_addon);
	INSERT_ADDON(1962, enhance_vit_addon);

	INSERT_ADDON(1963, enhance_eng_addon);
	INSERT_ADDON(1964, enhance_eng_addon);
	INSERT_ADDON(1965, enhance_eng_addon);
	INSERT_ADDON(1966, enhance_eng_addon);
	INSERT_ADDON(1967, enhance_eng_addon);
	INSERT_ADDON(1968, enhance_eng_addon);
	INSERT_ADDON(1969, enhance_eng_addon);
	INSERT_ADDON(1970, enhance_eng_addon);
	INSERT_ADDON(1971, enhance_eng_addon);
	
	INSERT_ADDON(1972, enhance_speed_addon_point);
	INSERT_ADDON(1973, enhance_speed_addon_point);
	INSERT_ADDON(1974, enhance_speed_addon_point);
	INSERT_ADDON(1975, enhance_speed_addon_point);
	INSERT_ADDON(1976, enhance_speed_addon_point);
	
	INSERT_ADDON(1977, enhance_crit_rate);
	INSERT_ADDON(1978, enhance_crit_rate);
	INSERT_ADDON(1979, enhance_crit_rate);
	INSERT_ADDON(1980, enhance_crit_rate);
	INSERT_ADDON(1981, enhance_crit_rate);

	INSERT_ADDON(1982, enhance_damage_reduce_addon);
	INSERT_ADDON(1983, enhance_damage_reduce_addon);
	INSERT_ADDON(1984, enhance_damage_reduce_addon);
	INSERT_ADDON(1985, enhance_damage_reduce_addon);
	INSERT_ADDON(1986, enhance_damage_reduce_addon);

	INSERT_ADDON(1987, enhance_hpgen_addon);
	INSERT_ADDON(1988, enhance_hpgen_addon);
	INSERT_ADDON(1989, enhance_hpgen_addon);
	INSERT_ADDON(1990, enhance_hpgen_addon);
	INSERT_ADDON(1991, enhance_hpgen_addon);

	INSERT_ADDON(1992, enhance_mpgen_addon);
	INSERT_ADDON(1993, enhance_mpgen_addon);
	INSERT_ADDON(1994, enhance_mpgen_addon);
	INSERT_ADDON(1995, enhance_mpgen_addon);
	INSERT_ADDON(1996, enhance_mpgen_addon);

	INSERT_ADDON(1997, enhance_hp_addon_2);
	INSERT_ADDON(1998, enhance_hp_addon_2);
	INSERT_ADDON(1999, enhance_hp_addon_2);
	INSERT_ADDON(2000, enhance_hp_addon_2);
	INSERT_ADDON(2001, enhance_hp_addon_2);

	INSERT_ADDON(2002, enhance_mp_addon_2);
	INSERT_ADDON(2003, enhance_mp_addon_2);
	INSERT_ADDON(2004, enhance_mp_addon_2);
	INSERT_ADDON(2005, enhance_mp_addon_2);
	INSERT_ADDON(2006, enhance_mp_addon_2);
                     
	INSERT_ADDON(2007, reduce_cast_time_addon);
	INSERT_ADDON(2008, reduce_cast_time_addon);
	INSERT_ADDON(2009, reduce_cast_time_addon);
	INSERT_ADDON(2010, reduce_cast_time_addon);
	INSERT_ADDON(2011, reduce_cast_time_addon);

	INSERT_ADDON(2012, enhance_attack_speed_addon);
	INSERT_ADDON(2013, enhance_attack_speed_addon);
	INSERT_ADDON(2014, enhance_attack_speed_addon);
                     
	INSERT_ADDON(2015, enhance_exp_addon);
	INSERT_ADDON(2016, enhance_exp_addon);
	INSERT_ADDON(2017, enhance_exp_addon);
	INSERT_ADDON(2018, enhance_exp_addon);
	INSERT_ADDON(2019, enhance_exp_addon);

	INSERT_ADDON(2020,enhance_damage_addon);
	INSERT_ADDON(2021,enhance_crit_rate);
	INSERT_ADDON(2024,enhance_crit_rate);
	INSERT_ADDON(2025,enhance_crit_rate);
	INSERT_ADDON(2027,enhance_crit_rate);
	INSERT_ADDON(2028,enhance_crit_rate);

	INSERT_ADDON(2022, enhance_max_damage_addon);
	INSERT_ADDON(2023, enhance_max_magic_addon);

	INSERT_ADDON(2029, enhance_attack_degree);
	INSERT_ADDON(2030, enhance_attack_degree);
	INSERT_ADDON(2031, enhance_attack_degree);
	INSERT_ADDON(2032, enhance_attack_degree);
	INSERT_ADDON(2033, enhance_attack_degree);
	INSERT_ADDON(2034, enhance_attack_degree);
	INSERT_ADDON(2035, enhance_attack_degree);
	INSERT_ADDON(2036, enhance_attack_degree);
	INSERT_ADDON(2037, enhance_attack_degree);
	INSERT_ADDON(2038, enhance_attack_degree);

	INSERT_ADDON(2039, enhance_defend_degree);
	INSERT_ADDON(2040, enhance_defend_degree);
	INSERT_ADDON(2041, enhance_defend_degree);
	INSERT_ADDON(2042, enhance_defend_degree);
	INSERT_ADDON(2043, enhance_defend_degree);
	INSERT_ADDON(2044, enhance_defend_degree);
	INSERT_ADDON(2045, enhance_defend_degree);
	INSERT_ADDON(2046, enhance_defend_degree);
	INSERT_ADDON(2047, enhance_defend_degree);
	INSERT_ADDON(2048, enhance_defend_degree);
	INSERT_ADDON(2049, enhance_defend_degree);
	INSERT_ADDON(2050, enhance_defend_degree);
	INSERT_ADDON(2051, enhance_defend_degree);

	INSERT_ADDON(2052, enhance_defend_degree);
	INSERT_ADDON(2053, enhance_defend_degree);
	INSERT_ADDON(2054, enhance_defend_degree);
	INSERT_ADDON(2055, enhance_defend_degree);
	INSERT_ADDON(2056, enhance_defend_degree);
	INSERT_ADDON(2057, enhance_defend_degree);
	INSERT_ADDON(2058, enhance_defend_degree);
	INSERT_ADDON(2059, enhance_defend_degree);
	INSERT_ADDON(2060, enhance_defend_degree);
	INSERT_ADDON(2061, enhance_defend_degree);
	INSERT_ADDON(2062, enhance_defend_degree);
	INSERT_ADDON(2063, enhance_defend_degree);
	INSERT_ADDON(2064, enhance_defend_degree);

	INSERT_ADDON(2065, enhance_attack_degree);
	INSERT_ADDON(2066, enhance_attack_degree);
	INSERT_ADDON(2067, enhance_attack_degree);
	INSERT_ADDON(2068, enhance_attack_degree);
	INSERT_ADDON(2069, enhance_attack_degree);
	INSERT_ADDON(2070, enhance_attack_degree);
	INSERT_ADDON(2071, enhance_attack_degree);
	INSERT_ADDON(2072, enhance_attack_degree);
	INSERT_ADDON(2073, enhance_attack_degree);
	INSERT_ADDON(2074, enhance_attack_degree);

	INSERT_ADDON(2075, enhance_attack_degree);
	INSERT_ADDON(2076, enhance_attack_degree);
	INSERT_ADDON(2077, enhance_attack_degree);
	INSERT_ADDON(2078, enhance_attack_degree);
	INSERT_ADDON(2079, enhance_attack_degree);
	INSERT_ADDON(2080, enhance_attack_degree);
	INSERT_ADDON(2081, enhance_attack_degree);
	INSERT_ADDON(2082, enhance_attack_degree);
	INSERT_ADDON(2083, enhance_attack_degree);
	INSERT_ADDON(2084, enhance_attack_degree);

	INSERT_ADDON(2085, enhance_defend_degree);
	INSERT_ADDON(2086, enhance_defend_degree);
	INSERT_ADDON(2087, enhance_defend_degree);
	INSERT_ADDON(2088, enhance_defend_degree);
	INSERT_ADDON(2089, enhance_defend_degree);
	INSERT_ADDON(2090, enhance_defend_degree);
	INSERT_ADDON(2091, enhance_defend_degree);
	INSERT_ADDON(2092, enhance_defend_degree);
	INSERT_ADDON(2093, enhance_defend_degree);
	INSERT_ADDON(2094, enhance_defend_degree);
	INSERT_ADDON(2095, enhance_defend_degree);
	INSERT_ADDON(2096, enhance_defend_degree);
	INSERT_ADDON(2097, enhance_defend_degree);

	INSERT_ADDON(2098, enhance_attack_degree);
	INSERT_ADDON(2099, enhance_attack_degree);
	INSERT_ADDON(2100, enhance_attack_degree);
	INSERT_ADDON(2101, enhance_attack_degree);

	INSERT_ADDON(2102, enhance_damage_reduce_addon);
	INSERT_ADDON(2103, enhance_damage_reduce_addon);
	INSERT_ADDON(2104, enhance_damage_reduce_addon);
	INSERT_ADDON(2105, enhance_damage_reduce_addon);
	INSERT_ADDON(2106, enhance_damage_reduce_addon);
	INSERT_ADDON(2107, enhance_damage_reduce_addon);
	INSERT_ADDON(2108, enhance_damage_reduce_addon);

	INSERT_ADDON(2109, item_armor_enhance_all_resistance);
	INSERT_ADDON(2110, item_armor_enhance_all_resistance);
	INSERT_ADDON(2111, item_armor_enhance_all_resistance);
	INSERT_ADDON(2112, item_armor_enhance_all_resistance);
	INSERT_ADDON(2113, item_armor_enhance_all_resistance);
	INSERT_ADDON(2114, item_armor_enhance_all_resistance);
	INSERT_ADDON(2115, item_armor_enhance_all_resistance);
	INSERT_ADDON(2116, item_armor_enhance_all_resistance);

	INSERT_ADDON(2117, enhance_mp_addon_2);
	INSERT_ADDON(2118, enhance_mp_addon_2);
	INSERT_ADDON(2119, enhance_mp_addon_2);
	INSERT_ADDON(2120, enhance_mp_addon_2);
	INSERT_ADDON(2121, enhance_mp_addon_2);
	INSERT_ADDON(2122, enhance_mp_addon_2);
	INSERT_ADDON(2123, enhance_mp_addon_2);
	INSERT_ADDON(2124, enhance_mp_addon_2);
	INSERT_ADDON(2125, enhance_mp_addon_2);

	INSERT_ADDON(2126, enhance_hp_addon_2);
	INSERT_ADDON(2127, enhance_hp_addon_2);
	INSERT_ADDON(2128, enhance_hp_addon_2);
	INSERT_ADDON(2129, enhance_hp_addon_2);
	INSERT_ADDON(2130, enhance_hp_addon_2);
	INSERT_ADDON(2131, enhance_hp_addon_2);
	INSERT_ADDON(2132, enhance_hp_addon_2);
	
	INSERT_ADDON(2133, enhance_hp_scale_addon);
	INSERT_ADDON(2134, enhance_hp_scale_addon);
	INSERT_ADDON(2135, enhance_hp_scale_addon);
	INSERT_ADDON(2136, enhance_hp_scale_addon);
	INSERT_ADDON(2137, enhance_hp_scale_addon);
	INSERT_ADDON(2138, enhance_hp_scale_addon);
	INSERT_ADDON(2139, enhance_hp_scale_addon);
	INSERT_ADDON(2140, enhance_hp_scale_addon);
	INSERT_ADDON(2141, enhance_hp_scale_addon);

	INSERT_ADDON(2142, enhance_attack_degree);
	INSERT_ADDON(2143, enhance_attack_degree);

	INSERT_ADDON(2144, enhance_defend_degree);
	INSERT_ADDON(2145, enhance_defend_degree);

	INSERT_ADDON(2146, enhance_vit_addon);
	INSERT_ADDON(2147, enhance_crit_rate);
	INSERT_ADDON(2148, reduce_cast_time_addon);


	INSERT_ADDON(2149, enhance_attack_degree);
	INSERT_ADDON(2150, enhance_attack_degree);
	INSERT_ADDON(2151, enhance_attack_degree);

	INSERT_ADDON(2154, enhance_defend_degree);
	INSERT_ADDON(2155, enhance_defend_degree);
	INSERT_ADDON(2156, enhance_defend_degree);
	INSERT_ADDON(2157, enhance_defend_degree);

	INSERT_ADDON(2167,enhance_defense_scale_addon_2);
	INSERT_ADDON(2168,enhance_defense_scale_addon_2);
	INSERT_ADDON(2169,enhance_defense_scale_addon_2);
	INSERT_ADDON(2170,enhance_defense_scale_addon_2);
	INSERT_ADDON(2171,enhance_defense_scale_addon_2);
	INSERT_ADDON(2172,enhance_defense_scale_addon_2);
	INSERT_ADDON(2173,enhance_defense_scale_addon_2);
	INSERT_ADDON(2174,enhance_defense_scale_addon_2);
	INSERT_ADDON(2175,enhance_defense_scale_addon_2);
	
	INSERT_ADDON(2176,enhance_resistance3_scale_addon);
	INSERT_ADDON(2177,enhance_resistance3_scale_addon);
	INSERT_ADDON(2178,enhance_resistance3_scale_addon);
	INSERT_ADDON(2179,enhance_resistance3_scale_addon);
	INSERT_ADDON(2180,enhance_resistance3_scale_addon);
	INSERT_ADDON(2181,enhance_resistance3_scale_addon);
	INSERT_ADDON(2182,enhance_resistance3_scale_addon);
	INSERT_ADDON(2183,enhance_resistance3_scale_addon);
	INSERT_ADDON(2184,enhance_resistance3_scale_addon);

	INSERT_ADDON(2185,enhance_resistance1_scale_addon);
	INSERT_ADDON(2186,enhance_resistance1_scale_addon);
	INSERT_ADDON(2187,enhance_resistance1_scale_addon);
	INSERT_ADDON(2188,enhance_resistance1_scale_addon);
	INSERT_ADDON(2189,enhance_resistance1_scale_addon);
	INSERT_ADDON(2190,enhance_resistance1_scale_addon);
	INSERT_ADDON(2191,enhance_resistance1_scale_addon);
	INSERT_ADDON(2192,enhance_resistance1_scale_addon);
	INSERT_ADDON(2193,enhance_resistance1_scale_addon);

	INSERT_ADDON(2194,enhance_resistance0_scale_addon);
	INSERT_ADDON(2195,enhance_resistance0_scale_addon);
	INSERT_ADDON(2196,enhance_resistance0_scale_addon);
	INSERT_ADDON(2197,enhance_resistance0_scale_addon);
	INSERT_ADDON(2198,enhance_resistance0_scale_addon);
	INSERT_ADDON(2199,enhance_resistance0_scale_addon);
	INSERT_ADDON(2200,enhance_resistance0_scale_addon);
	INSERT_ADDON(2201,enhance_resistance0_scale_addon);
	INSERT_ADDON(2202,enhance_resistance0_scale_addon);

	INSERT_ADDON(2203,enhance_resistance4_scale_addon);
	INSERT_ADDON(2204,enhance_resistance4_scale_addon);
	INSERT_ADDON(2205,enhance_resistance4_scale_addon);
	INSERT_ADDON(2206,enhance_resistance4_scale_addon);
	INSERT_ADDON(2207,enhance_resistance4_scale_addon);
	INSERT_ADDON(2208,enhance_resistance4_scale_addon);
	INSERT_ADDON(2209,enhance_resistance4_scale_addon);
	INSERT_ADDON(2210,enhance_resistance4_scale_addon);
	INSERT_ADDON(2211,enhance_resistance4_scale_addon);

	INSERT_ADDON(2212,enhance_resistance2_scale_addon);
	INSERT_ADDON(2213,enhance_resistance2_scale_addon);
	INSERT_ADDON(2214,enhance_resistance2_scale_addon);
	INSERT_ADDON(2215,enhance_resistance2_scale_addon);
	INSERT_ADDON(2216,enhance_resistance2_scale_addon);
	INSERT_ADDON(2217,enhance_resistance2_scale_addon);
	INSERT_ADDON(2218,enhance_resistance2_scale_addon);
	INSERT_ADDON(2219,enhance_resistance2_scale_addon);
	INSERT_ADDON(2220,enhance_resistance2_scale_addon);

	INSERT_ADDON(2221,enhance_damage_scale_addon_2);
	INSERT_ADDON(2222,enhance_damage_scale_addon_2);
	INSERT_ADDON(2223,enhance_damage_scale_addon_2);
	INSERT_ADDON(2224,enhance_damage_scale_addon_2);
	INSERT_ADDON(2225,enhance_damage_scale_addon_2);
	INSERT_ADDON(2226,enhance_damage_scale_addon_2);
	INSERT_ADDON(2227,enhance_damage_scale_addon_2);
	INSERT_ADDON(2228,enhance_damage_scale_addon_2);
	INSERT_ADDON(2229,enhance_damage_scale_addon_2);

	INSERT_ADDON(2230,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2231,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2232,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2233,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2234,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2235,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2236,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2237,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2238,enhance_magic_damage_scale_addon);

	INSERT_ADDON(2239,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2240,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2241,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2242,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2243,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2244,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2245,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2246,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2247,enhance_all_resistance_scale_addon);

	INSERT_ADDON(2248,enhance_damage_scale_addon_2);
	INSERT_ADDON(2249,enhance_damage_scale_addon_2);
	INSERT_ADDON(2250,enhance_damage_scale_addon_2);
	INSERT_ADDON(2251,enhance_damage_scale_addon_2);
	INSERT_ADDON(2252,enhance_damage_scale_addon_2);
	INSERT_ADDON(2253,enhance_damage_scale_addon_2);
	INSERT_ADDON(2254,enhance_damage_scale_addon_2);
	INSERT_ADDON(2255,enhance_damage_scale_addon_2);
	INSERT_ADDON(2256,enhance_damage_scale_addon_2);

	INSERT_ADDON(2257,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2258,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2259,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2260,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2261,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2262,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2265,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2266,enhance_magic_damage_scale_addon);
	INSERT_ADDON(2267,enhance_magic_damage_scale_addon);

	INSERT_ADDON(2270, enhance_eng_addon);
	INSERT_ADDON(2271, enhance_agi_addon);
	INSERT_ADDON(2272, enhance_str_addon);
	INSERT_ADDON(2273, enhance_vit_addon);
	INSERT_ADDON(2275, item_skill_addon);
	INSERT_ADDON(2276, item_skill_addon);
	INSERT_ADDON(2277, item_skill_addon);
	INSERT_ADDON(2278, item_skill_addon);
	INSERT_ADDON(2279, item_skill_addon);
	INSERT_ADDON(2280, item_skill_addon);
	INSERT_ADDON(2281, item_skill_addon);
	INSERT_ADDON(2282, item_skill_addon);
	INSERT_ADDON(2283, item_skill_addon);
	
	INSERT_ADDON(2472, item_skill_addon);
	INSERT_ADDON(2473, item_skill_addon);
	INSERT_ADDON(2474, item_skill_addon);
	INSERT_ADDON(2475, item_skill_addon);

	INSERT_ADDON(2477, item_rebound_skill_addon);
	INSERT_ADDON(2479, item_rebound_skill_addon);
	
	
	INSERT_ADDON(2299,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2300,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2301,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2302,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2303,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2304,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2305,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2306,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2307,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2308,enhance_all_resistance_scale_addon);
	INSERT_ADDON(2309,enhance_defend_degree);
	INSERT_ADDON(2310,enhance_defend_degree);
	INSERT_ADDON(2311,query_other_property_addon);
	INSERT_ADDON(2362,enhance_soulpower_addon);
	
	INSERT_ADDON(2447,enhance_damage_addon);
	INSERT_ADDON(2448,enhance_defense_addon_1arg);
	INSERT_ADDON(2449,enhance_damage_addon);
	INSERT_ADDON(2450,enhance_defense_addon_1arg);
	INSERT_ADDON(2451,enhance_damage_addon);
	INSERT_ADDON(2452,enhance_defense_addon_1arg);
	INSERT_ADDON(2453,enhance_magic_damage_addon);
	INSERT_ADDON(2454,enhance_all_resistance_addon);
	INSERT_ADDON(2455,enhance_magic_damage_addon);
	INSERT_ADDON(2456,enhance_all_resistance_addon);
	INSERT_ADDON(2457,enhance_attack_addon);
	INSERT_ADDON(2458,enhance_armor_addon);
	INSERT_ADDON(2459,enhance_attack_addon);
	INSERT_ADDON(2460,enhance_armor_addon);
	INSERT_ADDON(2461,enhance_attack_addon);
	INSERT_ADDON(2462,enhance_armor_addon);
	INSERT_ADDON(2463,enhance_hp_addon);
	INSERT_ADDON(2464,enhance_hp_addon);
	INSERT_ADDON(2465,enhance_vit_addon);
	INSERT_ADDON(2466,enhance_magic_damage_addon);
	INSERT_ADDON(2467,enhance_all_resistance_addon);
	INSERT_ADDON(2482, item_skill_addon);

	INSERT_ADDON(2483,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2484,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2485,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2486,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2487,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2488,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2489,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2490,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2491,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2492,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2493,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2494,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2495,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2496,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2497,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2498,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2499,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2500,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2501,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2502,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2503,enhance_all_magic_damage_reduce_addon);
	INSERT_ADDON(2504,enhance_all_magic_damage_reduce_addon);
	INSERT_ADDON(2505,enhance_all_magic_damage_reduce_addon);

	INSERT_ADDON(2506,enhance_resistance1_addon);
	INSERT_ADDON(2507,enhance_resistance1_addon);
	INSERT_ADDON(2508,enhance_resistance1_addon);
	INSERT_ADDON(2509,enhance_resistance1_addon);
	INSERT_ADDON(2510,enhance_resistance1_addon);
	INSERT_ADDON(2511,enhance_resistance1_addon);
	INSERT_ADDON(2512,enhance_resistance2_addon);
	INSERT_ADDON(2513,enhance_resistance2_addon);
	INSERT_ADDON(2514,enhance_resistance2_addon);
	INSERT_ADDON(2515,enhance_resistance2_addon);
	INSERT_ADDON(2516,enhance_resistance2_addon);
	INSERT_ADDON(2517,enhance_resistance2_addon);
	INSERT_ADDON(2518,enhance_resistance0_addon);
	INSERT_ADDON(2519,enhance_resistance0_addon);
	INSERT_ADDON(2520,enhance_resistance0_addon);
	INSERT_ADDON(2521,enhance_resistance0_addon);
	INSERT_ADDON(2522,enhance_resistance0_addon);
	INSERT_ADDON(2523,enhance_resistance0_addon);
	INSERT_ADDON(2524,enhance_resistance4_addon);
	INSERT_ADDON(2525,enhance_resistance4_addon);
	INSERT_ADDON(2526,enhance_resistance4_addon);
	INSERT_ADDON(2527,enhance_resistance4_addon);
	INSERT_ADDON(2528,enhance_resistance4_addon);
	INSERT_ADDON(2529,enhance_resistance4_addon);
	INSERT_ADDON(2530,enhance_resistance3_addon);
	INSERT_ADDON(2531,enhance_resistance3_addon);
	INSERT_ADDON(2532,enhance_resistance3_addon);
	INSERT_ADDON(2533,enhance_resistance3_addon);
	INSERT_ADDON(2534,enhance_resistance3_addon);
	INSERT_ADDON(2535,enhance_resistance3_addon);
	
	INSERT_ADDON(2536,enhance_attack_addon);
	INSERT_ADDON(2537,enhance_attack_addon);
	INSERT_ADDON(2538,enhance_attack_addon);
	INSERT_ADDON(2539,enhance_attack_addon);
	INSERT_ADDON(2540,enhance_attack_addon);
	INSERT_ADDON(2541,enhance_attack_addon);
	INSERT_ADDON(2542,enhance_attack_addon);
	INSERT_ADDON(2543,enhance_armor_addon_2);
	INSERT_ADDON(2544,enhance_armor_addon_2);
	INSERT_ADDON(2545,enhance_armor_addon_2);
	INSERT_ADDON(2546,enhance_armor_addon_2);
	INSERT_ADDON(2547,enhance_armor_addon_2);
	INSERT_ADDON(2548,enhance_armor_addon_2);
	INSERT_ADDON(2549,enhance_armor_addon_2);
	INSERT_ADDON(2550,enhance_mp_addon);
	INSERT_ADDON(2551,enhance_mp_addon);
	INSERT_ADDON(2552,enhance_mp_addon);
	INSERT_ADDON(2553,enhance_mp_addon);
	INSERT_ADDON(2554,enhance_mp_addon);
	INSERT_ADDON(2555,enhance_mp_addon);
	INSERT_ADDON(2556,enhance_mp_addon);
	INSERT_ADDON(2557,enhance_mp_addon);
	INSERT_ADDON(2558,enhance_vit_addon);
	INSERT_ADDON(2559,enhance_vit_addon);
	INSERT_ADDON(2560,enhance_vit_addon);
	INSERT_ADDON(2561,enhance_vit_addon);
	INSERT_ADDON(2562,enhance_vit_addon);
	INSERT_ADDON(2563,enhance_vit_addon);
	INSERT_ADDON(2564,enhance_vit_addon);
	INSERT_ADDON(2565,enhance_vit_addon);
	INSERT_ADDON(2566,enhance_vit_addon);
	INSERT_ADDON(2567,enhance_str_addon);
	INSERT_ADDON(2568,enhance_str_addon);
	INSERT_ADDON(2569,enhance_str_addon);
	INSERT_ADDON(2570,enhance_str_addon);
	INSERT_ADDON(2571,enhance_str_addon);
	INSERT_ADDON(2572,enhance_str_addon);
	INSERT_ADDON(2573,enhance_str_addon);
	INSERT_ADDON(2574,enhance_str_addon);
	INSERT_ADDON(2575,enhance_str_addon);
	INSERT_ADDON(2576,enhance_agi_addon);
	INSERT_ADDON(2577,enhance_agi_addon);
	INSERT_ADDON(2578,enhance_agi_addon);
	INSERT_ADDON(2579,enhance_agi_addon);
	INSERT_ADDON(2580,enhance_agi_addon);
	INSERT_ADDON(2581,enhance_agi_addon);
	INSERT_ADDON(2582,enhance_agi_addon);
	INSERT_ADDON(2583,enhance_agi_addon);
	INSERT_ADDON(2584,enhance_agi_addon);
	INSERT_ADDON(2585,enhance_eng_addon);
	INSERT_ADDON(2586,enhance_eng_addon);
	INSERT_ADDON(2587,enhance_eng_addon);
	INSERT_ADDON(2588,enhance_eng_addon);
	INSERT_ADDON(2589,enhance_eng_addon);
	INSERT_ADDON(2590,enhance_eng_addon);
	INSERT_ADDON(2591,enhance_eng_addon);
	INSERT_ADDON(2592,enhance_eng_addon);
	INSERT_ADDON(2593,enhance_eng_addon);
	INSERT_ADDON(2594,enhance_hp_addon);
	INSERT_ADDON(2595,enhance_hp_addon);
	INSERT_ADDON(2596,enhance_hp_addon);
	INSERT_ADDON(2597,enhance_hp_addon);
	INSERT_ADDON(2598,enhance_hp_addon);
	INSERT_ADDON(2599,enhance_hp_addon);
	INSERT_ADDON(2600,enhance_hp_addon);
	INSERT_ADDON(2601,enhance_hp_addon);
	INSERT_ADDON(2602,enhance_hp_addon);
	INSERT_ADDON(2603,enhance_hp_addon);
	INSERT_ADDON(2604,enhance_hp_addon);
	INSERT_ADDON(2605,enhance_hp_addon);
	INSERT_ADDON(2606,enhance_hp_addon);
	INSERT_ADDON(2607,enhance_hp_addon);
	INSERT_ADDON(2608,enhance_hp_addon);
	INSERT_ADDON(2609,enhance_hp_addon);
	INSERT_ADDON(2610,enhance_hp_addon);
	INSERT_ADDON(2611,enhance_hp_addon);
	INSERT_ADDON(2612,enhance_hp_addon);
	INSERT_ADDON(2613,enhance_hp_addon);
	INSERT_ADDON(2614,enhance_hp_addon);
	INSERT_ADDON(2615,enhance_hp_addon);
	INSERT_ADDON(2616,enhance_defense_addon_2);
	INSERT_ADDON(2617,enhance_defense_addon_2);
	INSERT_ADDON(2618,enhance_defense_addon_2);
	INSERT_ADDON(2619,enhance_defense_addon_2);
	INSERT_ADDON(2620,enhance_defense_addon_2);
	INSERT_ADDON(2621,enhance_defense_addon_2);
	INSERT_ADDON(2622,enhance_defense_addon_2);
	INSERT_ADDON(2623,enhance_defense_addon_2);
	INSERT_ADDON(2624,enhance_defense_addon_2);
	INSERT_ADDON(2625,enhance_defense_addon_2);
	INSERT_ADDON(2626,enhance_defense_addon_2);
	INSERT_ADDON(2627,enhance_defense_addon_2);
	INSERT_ADDON(2628,enhance_defense_addon_2);
	INSERT_ADDON(2629,enhance_defense_addon_2);
	INSERT_ADDON(2630,enhance_defense_addon_2);
	INSERT_ADDON(2631,enhance_defense_addon_2);
	INSERT_ADDON(2632,enhance_defense_addon_2);
	INSERT_ADDON(2633,enhance_all_resistance_addon);
	INSERT_ADDON(2634,enhance_all_resistance_addon);
	INSERT_ADDON(2635,enhance_all_resistance_addon);
	INSERT_ADDON(2636,enhance_all_resistance_addon);
	INSERT_ADDON(2637,enhance_all_resistance_addon);
	INSERT_ADDON(2638,enhance_all_resistance_addon);
	INSERT_ADDON(2639,enhance_all_resistance_addon);
	INSERT_ADDON(2640,enhance_all_resistance_addon);
	INSERT_ADDON(2641,enhance_all_resistance_addon);
	INSERT_ADDON(2642,enhance_all_resistance_addon);
	INSERT_ADDON(2643,enhance_all_resistance_addon);
	INSERT_ADDON(2644,enhance_all_resistance_addon);
	INSERT_ADDON(2645,enhance_all_resistance_addon);
	INSERT_ADDON(2646,enhance_all_resistance_addon);
	INSERT_ADDON(2647,enhance_all_resistance_addon);
	INSERT_ADDON(2648,enhance_all_resistance_addon);
	INSERT_ADDON(2649,enhance_all_resistance_addon);
	INSERT_ADDON(2650,enhance_damage_addon_2);
	INSERT_ADDON(2651,enhance_damage_addon_2);
	INSERT_ADDON(2652,enhance_damage_addon_2);
	INSERT_ADDON(2653,enhance_damage_addon_2);
	INSERT_ADDON(2654,enhance_damage_addon_2);
	INSERT_ADDON(2655,enhance_damage_addon_2);
	INSERT_ADDON(2656,enhance_damage_addon_2);
	INSERT_ADDON(2657,enhance_damage_addon_2);
	INSERT_ADDON(2658,enhance_damage_addon_2);
	INSERT_ADDON(2659,enhance_damage_addon_2);
	INSERT_ADDON(2660,enhance_damage_addon_2);
	INSERT_ADDON(2661,enhance_damage_addon_2);
	INSERT_ADDON(2662,enhance_damage_addon_2);
	INSERT_ADDON(2663,enhance_damage_addon_2);
	INSERT_ADDON(2664,enhance_damage_addon_2);
	INSERT_ADDON(2665,enhance_damage_addon_2);
	INSERT_ADDON(2666,enhance_damage_addon_2);
	INSERT_ADDON(2667,enhance_damage_addon_2);
	INSERT_ADDON(2668,enhance_magic_damage_addon_2);
	INSERT_ADDON(2669,enhance_magic_damage_addon_2);
	INSERT_ADDON(2670,enhance_magic_damage_addon_2);
	INSERT_ADDON(2671,enhance_magic_damage_addon_2);
	INSERT_ADDON(2672,enhance_magic_damage_addon_2);
	INSERT_ADDON(2673,enhance_magic_damage_addon_2);
	INSERT_ADDON(2674,enhance_magic_damage_addon_2);
	INSERT_ADDON(2675,enhance_magic_damage_addon_2);
	INSERT_ADDON(2676,enhance_magic_damage_addon_2);
	INSERT_ADDON(2677,enhance_magic_damage_addon_2);
	INSERT_ADDON(2678,enhance_magic_damage_addon_2);
	INSERT_ADDON(2679,enhance_magic_damage_addon_2);
	INSERT_ADDON(2680,enhance_magic_damage_addon_2);
	INSERT_ADDON(2681,enhance_magic_damage_addon_2);
	INSERT_ADDON(2682,enhance_magic_damage_addon_2);
	INSERT_ADDON(2683,enhance_magic_damage_addon_2);
	INSERT_ADDON(2684,enhance_magic_damage_addon_2);
	INSERT_ADDON(2685,enhance_magic_damage_addon_2);
	INSERT_ADDON(2686,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2687,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2688,enhance_magic_damage_reduce0_addon);
	INSERT_ADDON(2689,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2690,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2691,enhance_magic_damage_reduce1_addon);
	INSERT_ADDON(2692,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2693,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2694,enhance_magic_damage_reduce2_addon);
	INSERT_ADDON(2695,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2696,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2697,enhance_magic_damage_reduce3_addon);
	INSERT_ADDON(2698,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2699,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2700,enhance_magic_damage_reduce4_addon);
	INSERT_ADDON(2701,enhance_crit_rate);
	INSERT_ADDON(2702,enhance_attack_degree);
	INSERT_ADDON(2703,enhance_attack_degree);
	INSERT_ADDON(2704,enhance_defend_degree);
	INSERT_ADDON(2705,enhance_defend_degree);
	INSERT_ADDON(2706,enhance_damage_reduce_addon);
	INSERT_ADDON(2707,enhance_damage_reduce_addon);
	INSERT_ADDON(2708,enhance_all_magic_damage_reduce_addon);
	INSERT_ADDON(2709,enhance_all_magic_damage_reduce_addon);

	INSERT_ADDON(2710,item_skill_addon);
	INSERT_ADDON(2711,item_skill_addon);
	INSERT_ADDON(2712,item_skill_addon);
	INSERT_ADDON(2713,item_skill_addon);
	INSERT_ADDON(2714,item_skill_addon);
	INSERT_ADDON(2715,item_rebound_skill_addon);
	INSERT_ADDON(2716,item_rebound_skill_addon);
	
	INSERT_ADDON(2717,enhance_eng_addon);
	INSERT_ADDON(2718,enhance_str_addon);
	INSERT_ADDON(2719,enhance_agi_addon);
	INSERT_ADDON(2720,enhance_vit_addon);
	INSERT_ADDON(2721,enhance_attack_degree_2arg);
	INSERT_ADDON(2722,enhance_attack_degree_2arg);
	INSERT_ADDON(2723,enhance_defend_degree_2arg);
	INSERT_ADDON(2724,enhance_defend_degree_2arg);
	INSERT_ADDON(2725,enhance_crit_rate_2arg);
	INSERT_ADDON(2726,enhance_crit_rate_2arg);
	INSERT_ADDON(2727,enhance_damage_addon_2arg);
	INSERT_ADDON(2728,enhance_magic_damage_addon_2arg);
	INSERT_ADDON(2729,enhance_hp_addon_2);
	INSERT_ADDON(2730,enhance_mp_addon_2);
	INSERT_ADDON(2731,enhance_attack_scale_addon_2arg);
	INSERT_ADDON(2732,enhance_defense_addon_2_2arg);
	INSERT_ADDON(2733,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(2734,enhance_damage_reduce_addon_2arg);
	INSERT_ADDON(2735,enhance_all_magic_damage_reduce_addon_2arg);
	INSERT_ADDON(2736,reduce_cast_time_addon_2arg);
	INSERT_ADDON(2737,reduce_cast_time_addon_2arg);
	INSERT_ADDON(2738,reduce_require_addon);
	INSERT_ADDON(2739,enhance_attack_range_addon_2arg);

	INSERT_ADDON(2740,enhance_hp_addon_2);
	INSERT_ADDON(2741,enhance_hp_addon_2);
	INSERT_ADDON(2742,enhance_mp_addon_2);
	INSERT_ADDON(2743,enhance_mpgen_addon_2arg);
	INSERT_ADDON(2744,enhance_eng_addon);
	INSERT_ADDON(2745,enhance_str_addon);
	INSERT_ADDON(2746,enhance_agi_addon);
	INSERT_ADDON(2747,enhance_vit_addon);
	INSERT_ADDON(2748,enhance_defense_scale_addon_2_2arg);
	INSERT_ADDON(2749,enhance_all_resistance_scale_addon_2arg);
	INSERT_ADDON(2750,enhance_hpgen_addon_2arg);
	INSERT_ADDON(2751,enhance_armor_addon_2_2arg);
	INSERT_ADDON(2752,reduce_cast_time_addon_2arg);
	INSERT_ADDON(2753,reduce_cast_time_addon_2arg);
	INSERT_ADDON(2754,enhance_crit_rate_2arg);
	INSERT_ADDON(2755,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(2756,enhance_defense_addon_2_2arg);
	INSERT_ADDON(2757,enhance_damage_reduce_addon_2arg);
	INSERT_ADDON(2758,enhance_all_magic_damage_reduce_addon_2arg);

	INSERT_ADDON(2839,enhance_exp_addon);
	INSERT_ADDON(2840,enhance_exp_addon);
	INSERT_ADDON(2841,enhance_exp_addon);
	INSERT_ADDON(2842,enhance_hp_scale_addon);

	INSERT_ADDON(2843,enhance_penetration);
	INSERT_ADDON(2844,enhance_penetration);
	INSERT_ADDON(2845,enhance_penetration);
	INSERT_ADDON(2846,enhance_penetration);
	INSERT_ADDON(2847,enhance_penetration);
	INSERT_ADDON(2848,enhance_penetration);
	INSERT_ADDON(2849,enhance_penetration);
	INSERT_ADDON(2850,enhance_resilience);
	INSERT_ADDON(2851,enhance_resilience);
	INSERT_ADDON(2852,enhance_resilience);
	INSERT_ADDON(2853,enhance_resilience);
	INSERT_ADDON(2854,enhance_resilience);
	INSERT_ADDON(2855,enhance_resilience);
	INSERT_ADDON(2856,enhance_resilience);
	INSERT_ADDON(2857,enhance_resilience);
	INSERT_ADDON(2858,enhance_resilience);
	INSERT_ADDON(2859,enhance_resilience);
	
	INSERT_ADDON(2881,enhance_penetration);
	INSERT_ADDON(2882,enhance_magic_damage_addon);
	INSERT_ADDON(2883,enhance_all_resistance_addon);
	INSERT_ADDON(2884,enhance_damage_addon);
	INSERT_ADDON(2885,enhance_defense_addon_1arg);
	INSERT_ADDON(2886,enhance_hp_addon);
	
	INSERT_ADDON(2887,enhance_resilience);
	INSERT_ADDON(2888,enhance_resilience);
	INSERT_ADDON(2889,enhance_penetration);
	INSERT_ADDON(2890,enhance_penetration);

typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,max_hp),DOUBLE_POINT> enhance_hp_addon_2;
typedef EPSA_addon<int, POINT_OFF+offsetof(enhanced_param,max_mp),DOUBLE_POINT> enhance_mp_addon_2;
/*
#define SET_ADDON_MACRO(value, type)  set_equip_addon<value,type>
	INSERT_ADDON(1506, SET_ADDON_MACRO(2,enhance_crit_rate));
	INSERT_ADDON(1507, SET_ADDON_MACRO(3,enhance_str_addon));
	INSERT_ADDON(1508, SET_ADDON_MACRO(4,item_skill_addon));
	*/
#define SET_ADDON_MACRO(value, type)  set_equip_addon<value,type>
	INSERT_ADDON(1895, SET_ADDON_MACRO(2,enhance_str_addon));
	INSERT_ADDON(1896, SET_ADDON_MACRO(4,enhance_damage_reduce_addon));

	INSERT_ADDON(1897, SET_ADDON_MACRO(2,enhance_agi_addon));
	INSERT_ADDON(1898, SET_ADDON_MACRO(4,enhance_damage_reduce_addon));

	INSERT_ADDON(1899, SET_ADDON_MACRO(2,enhance_eng_addon));
	INSERT_ADDON(1900, SET_ADDON_MACRO(4,enhance_damage_reduce_addon));

	INSERT_ADDON(1901, SET_ADDON_MACRO(2,enhance_attack_speed_addon));
	INSERT_ADDON(1902, SET_ADDON_MACRO(4,enhance_vit_addon));
	INSERT_ADDON(1903, SET_ADDON_MACRO(6,enhance_crit_rate));

	INSERT_ADDON(1904, SET_ADDON_MACRO(2,enhance_attack_speed_addon));
	INSERT_ADDON(1905, SET_ADDON_MACRO(4,enhance_vit_addon));
	INSERT_ADDON(1906, SET_ADDON_MACRO(6,enhance_crit_rate));

	INSERT_ADDON(1907, SET_ADDON_MACRO(2,reduce_cast_time_addon));
	INSERT_ADDON(1908, SET_ADDON_MACRO(4,enhance_vit_addon));
	INSERT_ADDON(1909, SET_ADDON_MACRO(6,enhance_crit_rate));

	INSERT_ADDON(2268, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2269, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2274, SET_ADDON_MACRO(6,enhance_defend_degree));
	
	INSERT_ADDON(2284, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2285, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2286, SET_ADDON_MACRO(6,enhance_defend_degree));
	
	INSERT_ADDON(2287, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2288, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2289, SET_ADDON_MACRO(6,enhance_defend_degree));
	
	INSERT_ADDON(2290, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2291, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2292, SET_ADDON_MACRO(6,enhance_defend_degree));
	
	INSERT_ADDON(2293, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2294, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2295, SET_ADDON_MACRO(6,enhance_defend_degree));
	
	INSERT_ADDON(2296, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2297, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2298, SET_ADDON_MACRO(6,enhance_defend_degree));
	
	INSERT_ADDON(2312, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2313, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2314, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2315, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2316, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2317, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2318, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2319, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2320, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2321, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2322, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2323, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2324, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2325, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2326, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2327, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2328, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2329, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2330, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2331, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2332, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2333, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2334, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2335, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2336, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2337, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2338, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2339, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2340, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2341, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2342, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2343, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2344, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2345, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2346, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2347, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2348, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2349, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2350, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2351, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2352, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2353, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2354, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2355, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2356, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2357, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2358, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2359, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2360, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2361, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2435, SET_ADDON_MACRO(2,enhance_defense_addon_2));
	INSERT_ADDON(2436, SET_ADDON_MACRO(3,enhance_all_resistance_addon));
	INSERT_ADDON(2437, SET_ADDON_MACRO(4,enhance_mp_addon));
	INSERT_ADDON(2438, SET_ADDON_MACRO(5,enhance_hp_addon));
	INSERT_ADDON(2439, SET_ADDON_MACRO(2,enhance_defense_addon_2));
	INSERT_ADDON(2440, SET_ADDON_MACRO(3,enhance_all_resistance_addon));
	INSERT_ADDON(2441, SET_ADDON_MACRO(4,enhance_mp_addon));
	INSERT_ADDON(2442, SET_ADDON_MACRO(5,enhance_hp_addon));
	INSERT_ADDON(2443, SET_ADDON_MACRO(2,enhance_defense_addon_2));
	INSERT_ADDON(2444, SET_ADDON_MACRO(3,enhance_all_resistance_addon));
	INSERT_ADDON(2445, SET_ADDON_MACRO(4,enhance_mp_addon));
	INSERT_ADDON(2446, SET_ADDON_MACRO(5,enhance_hp_addon));

	INSERT_ADDON(2468, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2469, SET_ADDON_MACRO(2,enhance_defense_addon_2));
	INSERT_ADDON(2470, SET_ADDON_MACRO(2,enhance_attack_addon));
	INSERT_ADDON(2471, SET_ADDON_MACRO(2,enhance_speed_addon_point));

	INSERT_ADDON(2759, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2760, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2761, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2762, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2763, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2764, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2765, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2766, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2767, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2768, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2769, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2770, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2771, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2772, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2773, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2774, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2775, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2776, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2777, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2778, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2779, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2780, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2781, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2782, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2783, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2784, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2785, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2786, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2787, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2788, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2789, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2790, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2791, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2792, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2793, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2794, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2795, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2796, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2797, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2798, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2799, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2800, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2801, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2802, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2803, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2804, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2805, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2806, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2807, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2808, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2809, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2810, SET_ADDON_MACRO(3,enhance_all_magic_damage_reduce_addon));
	INSERT_ADDON(2811, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2812, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2813, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2814, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2815, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2816, SET_ADDON_MACRO(3,enhance_attack_speed_addon));
	INSERT_ADDON(2817, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2818, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2819, SET_ADDON_MACRO(3,enhance_attack_speed_addon));
	INSERT_ADDON(2820, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2821, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2822, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2823, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2824, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2825, SET_ADDON_MACRO(3,reduce_cast_time_addon));
	INSERT_ADDON(2826, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2827, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2828, SET_ADDON_MACRO(3,enhance_defend_degree));
	INSERT_ADDON(2829, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2830, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2831, SET_ADDON_MACRO(3,enhance_damage_reduce_addon));
	INSERT_ADDON(2832, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2833, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2834, SET_ADDON_MACRO(3,enhance_soulpower_addon));
	INSERT_ADDON(2835, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2836, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2837, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2838, SET_ADDON_MACRO(4,enhance_attack_degree));
	INSERT_ADDON(2860, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2861, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(2862, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2863, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(2864, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2865, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(2866, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2867, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(2868, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2869, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(2870, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2871, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(2872, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2873, SET_ADDON_MACRO(4,enhance_penetration));
	INSERT_ADDON(2874, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2875, SET_ADDON_MACRO(4,enhance_penetration));
	INSERT_ADDON(2876, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(2877, SET_ADDON_MACRO(4,enhance_penetration));
	INSERT_ADDON(2878, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(2879, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(2880, SET_ADDON_MACRO(5,enhance_attack_speed_addon));

	INSERT_ADDON(2891, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2892, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2893, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2894, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2895, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2896, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2897, SET_ADDON_MACRO(2,enhance_hp_addon));
	INSERT_ADDON(2898, SET_ADDON_MACRO(5,enhance_defend_degree));
	INSERT_ADDON(2899, SET_ADDON_MACRO(6,enhance_attack_degree));
	INSERT_ADDON(2900, SET_ADDON_MACRO(2,reduce_cast_time_addon));
	INSERT_ADDON(2901, SET_ADDON_MACRO(3,enhance_eng_addon));
	INSERT_ADDON(2902, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2903, SET_ADDON_MACRO(5,enhance_attack_degree));
	INSERT_ADDON(2904, SET_ADDON_MACRO(2,enhance_crit_rate));
	INSERT_ADDON(2905, SET_ADDON_MACRO(3,enhance_agi_addon));
	INSERT_ADDON(2906, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2907, SET_ADDON_MACRO(5,enhance_attack_degree));
	INSERT_ADDON(2908, SET_ADDON_MACRO(2,enhance_crit_rate));
	INSERT_ADDON(2909, SET_ADDON_MACRO(3,enhance_str_addon));
	INSERT_ADDON(2910, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2911, SET_ADDON_MACRO(5,enhance_attack_degree));

#define TEMPORARY_ADDON_MACRO(type) temporary_addon_template<type>
	INSERT_ADDON(2363, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2364, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2365, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(2366, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2367, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2368, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2369, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2370, TEMPORARY_ADDON_MACRO(enhance_max_damage_addon));
	INSERT_ADDON(2371, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2372, TEMPORARY_ADDON_MACRO(enhance_max_magic_addon));
	INSERT_ADDON(2373, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(2374, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2375, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2376, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2377, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2378, TEMPORARY_ADDON_MACRO(enhance_max_damage_addon));
	INSERT_ADDON(2379, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2380, TEMPORARY_ADDON_MACRO(enhance_max_magic_addon));
	INSERT_ADDON(2381, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(2382, TEMPORARY_ADDON_MACRO(enhance_defense_addon));
	INSERT_ADDON(2383, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2384, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2385, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2386, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2387, TEMPORARY_ADDON_MACRO(enhance_max_damage_addon));
	INSERT_ADDON(2388, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2389, TEMPORARY_ADDON_MACRO(enhance_max_magic_addon));
	INSERT_ADDON(2390, TEMPORARY_ADDON_MACRO(enhance_defense_addon));
	INSERT_ADDON(2391, TEMPORARY_ADDON_MACRO(enhance_attack_addon_2));
	INSERT_ADDON(2392, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2393, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2394, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2395, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2396, TEMPORARY_ADDON_MACRO(enhance_max_damage_addon));
	INSERT_ADDON(2397, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2398, TEMPORARY_ADDON_MACRO(enhance_max_magic_addon));
	INSERT_ADDON(2399, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(2400, TEMPORARY_ADDON_MACRO(enhance_defense_addon));
	INSERT_ADDON(2401, TEMPORARY_ADDON_MACRO(enhance_attack_addon_2));
	INSERT_ADDON(2402, TEMPORARY_ADDON_MACRO(enhance_crit_rate));
	INSERT_ADDON(2403, TEMPORARY_ADDON_MACRO(enhance_attack_degree));
	INSERT_ADDON(2404, TEMPORARY_ADDON_MACRO(enhance_defend_degree));
	INSERT_ADDON(2405, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2406, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2407, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2408, TEMPORARY_ADDON_MACRO(enhance_max_damage_addon));
	INSERT_ADDON(2409, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2410, TEMPORARY_ADDON_MACRO(enhance_max_magic_addon));
	INSERT_ADDON(2411, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(2412, TEMPORARY_ADDON_MACRO(enhance_defense_addon));
	INSERT_ADDON(2413, TEMPORARY_ADDON_MACRO(enhance_attack_addon_2));
	INSERT_ADDON(2414, TEMPORARY_ADDON_MACRO(reduce_cast_time_addon));
	INSERT_ADDON(2415, TEMPORARY_ADDON_MACRO(enhance_attack_degree));
	INSERT_ADDON(2416, TEMPORARY_ADDON_MACRO(enhance_defend_degree));
	INSERT_ADDON(2417, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2418, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2419, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2420, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(2421, TEMPORARY_ADDON_MACRO(enhance_max_damage_addon));
 	INSERT_ADDON(2422, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(2423, TEMPORARY_ADDON_MACRO(enhance_max_magic_addon));
	INSERT_ADDON(2424, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(2425, TEMPORARY_ADDON_MACRO(enhance_defense_addon));
	INSERT_ADDON(2426, TEMPORARY_ADDON_MACRO(enhance_attack_addon_2));
	INSERT_ADDON(2427, TEMPORARY_ADDON_MACRO(enhance_crit_rate));
	INSERT_ADDON(2428, TEMPORARY_ADDON_MACRO(enhance_attack_degree));
	INSERT_ADDON(2429, TEMPORARY_ADDON_MACRO(enhance_defend_degree));
	INSERT_ADDON(2430, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2431, TEMPORARY_ADDON_MACRO(enhance_agi_addon));
	INSERT_ADDON(2432, TEMPORARY_ADDON_MACRO(enhance_eng_addon));
	INSERT_ADDON(2433, TEMPORARY_ADDON_MACRO(enhance_str_addon));
	INSERT_ADDON(2434, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	
	INSERT_ADDON(2912, enhance_attack_degree_2arg);
	INSERT_ADDON(2913, reduce_cast_time_addon_2arg);
	INSERT_ADDON(2914, reduce_cast_time_addon);
	INSERT_ADDON(2915, enhance_crit_rate);
	INSERT_ADDON(2916, enhance_damage_reduce_addon);
	INSERT_ADDON(2917, enhance_str_addon_1arg);
	INSERT_ADDON(2918, enhance_agi_addon_1arg);
	INSERT_ADDON(2919, enhance_eng_addon_1arg);
	INSERT_ADDON(2920, enhance_vit_addon_1arg);
	INSERT_ADDON(2921, enhance_all_magic_damage_reduce_addon);
	INSERT_ADDON(2922, enhance_soulpower_addon);
	INSERT_ADDON(2923, refine_defense_resistance);
	INSERT_ADDON(2974, enhance_attack_degree);
	INSERT_ADDON(2975, enhance_attack_degree);
	INSERT_ADDON(2976, enhance_attack_degree);
	INSERT_ADDON(2977, enhance_attack_degree);
	INSERT_ADDON(2978, enhance_attack_degree);
	INSERT_ADDON(2979, enhance_defend_degree);
	INSERT_ADDON(2980, enhance_defend_degree);
	INSERT_ADDON(2983, enhance_hp_addon);
	INSERT_ADDON(2984, enhance_mp_addon);
	INSERT_ADDON(2985, enhance_str_addon_1arg);
	INSERT_ADDON(2986, enhance_agi_addon_1arg);
	INSERT_ADDON(2987, enhance_eng_addon_1arg);

	INSERT_ADDON(2924, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2925, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2926, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2927, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(2928, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2929, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2930, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2931, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2932, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(2933, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2934, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2935, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2936, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2937, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(2938, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2939, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2940, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2941, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2942, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(2943, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2944, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2945, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2946, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2947, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(2948, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2949, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2950, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2951, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2952, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(2953, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2954, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2955, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2956, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2957, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(2958, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2959, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2960, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2961, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2962, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(2963, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2964, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2965, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2966, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2967, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(2968, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2969, SET_ADDON_MACRO(2,enhance_defend_degree));
	INSERT_ADDON(2970, SET_ADDON_MACRO(3,enhance_crit_rate));
	INSERT_ADDON(2971, SET_ADDON_MACRO(4,enhance_defend_degree));
	INSERT_ADDON(2972, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(2973, SET_ADDON_MACRO(6,enhance_attack_degree));

	INSERT_ADDON(2981, SET_ADDON_MACRO(2,enhance_attack_degree));
	INSERT_ADDON(2982, SET_ADDON_MACRO(2,enhance_penetration));
	
	
	INSERT_ADDON(2988, enhance_vit_addon);
	INSERT_ADDON(2989, enhance_hp_addon);
	INSERT_ADDON(2990, enhance_hp_addon);
	INSERT_ADDON(2991, enhance_hp_addon);
	INSERT_ADDON(2992, enhance_magic_damage_addon_2);
	INSERT_ADDON(2993, enhance_magic_damage_addon_2);
	INSERT_ADDON(2994, enhance_magic_damage_addon_2);
	INSERT_ADDON(2995, enhance_damage_addon_2);
	INSERT_ADDON(2996, enhance_damage_addon_2);
	INSERT_ADDON(2997, enhance_damage_addon_2);
	INSERT_ADDON(2998, enhance_all_resistance_addon);
	INSERT_ADDON(2999, enhance_all_resistance_addon);
	INSERT_ADDON(3000, enhance_all_resistance_addon);
	INSERT_ADDON(3001, enhance_defense_addon_1arg);
	INSERT_ADDON(3002, enhance_defense_addon_1arg);
	INSERT_ADDON(3003, enhance_defense_addon_1arg);

	INSERT_ADDON(3004, TEMPORARY_ADDON_MACRO(enhance_damage_addon));
	INSERT_ADDON(3005, TEMPORARY_ADDON_MACRO(enhance_magic_damage_addon));
	INSERT_ADDON(3006, TEMPORARY_ADDON_MACRO(enhance_hp_addon_2));
	INSERT_ADDON(3007, enhance_penetration);

	INSERT_ADDON(3008, enhance_defend_degree);
	INSERT_ADDON(3009, enhance_attack_degree);
	INSERT_ADDON(3010, enhance_str_addon_1arg);
	INSERT_ADDON(3011, enhance_str_addon);
	INSERT_ADDON(3012, enhance_eng_addon_1arg);
	INSERT_ADDON(3013, enhance_eng_addon);
	INSERT_ADDON(3014, enhance_agi_addon_1arg);
	INSERT_ADDON(3015, enhance_agi_addon);
	INSERT_ADDON(3016, enhance_vit_addon_1arg);
	INSERT_ADDON(3017, enhance_vit_addon);
	INSERT_ADDON(3018, enhance_hp_addon_2);
	INSERT_ADDON(3019, enhance_hp_addon);
	INSERT_ADDON(3020, enhance_crit_rate);
	INSERT_ADDON(3021, enhance_penetration);
	INSERT_ADDON(3022, enhance_resilience);
	INSERT_ADDON(3023, enhance_penetration);
	INSERT_ADDON(3024, enhance_resilience);
	INSERT_ADDON(3025, enhance_max_damage_addon_2_2arg);
	INSERT_ADDON(3026, enhance_max_damage_addon_2);
	INSERT_ADDON(3027, enhance_max_magic_addon_2_2arg);
	INSERT_ADDON(3028, enhance_max_magic_addon_2);
	INSERT_ADDON(3029, enhance_attack_addon);
	INSERT_ADDON(3030, enhance_armor_addon_2);
	INSERT_ADDON(3031, enhance_defense_addon_2_2arg);
	INSERT_ADDON(3032, enhance_defense_addon_2);
	INSERT_ADDON(3033, enhance_all_resistance_addon_2arg);
	INSERT_ADDON(3034, enhance_all_resistance_addon);
	INSERT_ADDON(3035, enhance_damage_addon_2_2arg);
	INSERT_ADDON(3036, enhance_damage_addon_2);
	INSERT_ADDON(3037, enhance_magic_damage_addon_2_2arg);
	INSERT_ADDON(3038, enhance_magic_damage_addon_2);
	INSERT_ADDON(3039, enhance_damage_addon_2_2arg);
	INSERT_ADDON(3040, enhance_damage_addon_2);
	INSERT_ADDON(3041, enhance_magic_damage_addon_2_2arg);
	INSERT_ADDON(3042, enhance_magic_damage_addon_2);
	INSERT_ADDON(3043, enhance_vigour);
	INSERT_ADDON(3044, enhance_vigour);
	INSERT_ADDON(3045, TEMPORARY_ADDON_MACRO(enhance_all_resistance_addon_2arg));
	INSERT_ADDON(3046, TEMPORARY_ADDON_MACRO(enhance_all_resistance_addon_2arg));
	INSERT_ADDON(3047, TEMPORARY_ADDON_MACRO(enhance_all_resistance_addon_2arg));
	INSERT_ADDON(3048, TEMPORARY_ADDON_MACRO(enhance_all_resistance_addon_2arg));
	INSERT_ADDON(3049, TEMPORARY_ADDON_MACRO(enhance_all_resistance_addon_2arg));
	INSERT_ADDON(3050,enhance_hp_addon_2);
	INSERT_ADDON(3051,enhance_hp_addon_2);
	INSERT_ADDON(3052,enhance_mp_addon_2);
	INSERT_ADDON(3053,enhance_mpgen_addon_2arg);
	INSERT_ADDON(3054,enhance_eng_addon);
	INSERT_ADDON(3055,enhance_str_addon);
	INSERT_ADDON(3056,enhance_agi_addon);
	INSERT_ADDON(3057,enhance_vit_addon);
	INSERT_ADDON(3058,enhance_defense_scale_addon_2_2arg);
	INSERT_ADDON(3059,enhance_all_resistance_scale_addon_2arg);
	INSERT_ADDON(3060,enhance_hpgen_addon_2arg);
	INSERT_ADDON(3061,enhance_armor_addon_2_2arg);
	INSERT_ADDON(3062,reduce_cast_time_addon_2arg);
	INSERT_ADDON(3063,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(3064,enhance_defense_addon_2_2arg);
	INSERT_ADDON(3065,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(3066,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(3067,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(3068,enhance_hp_addon_2);
	INSERT_ADDON(3069,enhance_hp_addon_2);
	INSERT_ADDON(3070,enhance_hp_addon_2);
	INSERT_ADDON(3071,enhance_eng_addon);
	INSERT_ADDON(3072,enhance_str_addon);
	INSERT_ADDON(3073,enhance_agi_addon);
	INSERT_ADDON(3074,enhance_vit_addon);
	INSERT_ADDON(3075,enhance_attack_degree_2arg);
	INSERT_ADDON(3076,enhance_attack_degree_2arg);
	INSERT_ADDON(3077,enhance_defend_degree_2arg);
	INSERT_ADDON(3078,enhance_defend_degree_2arg);
	INSERT_ADDON(3079,enhance_damage_addon_2arg);
	INSERT_ADDON(3080,enhance_magic_damage_addon_2arg);
	INSERT_ADDON(3081,enhance_hp_addon_2);
	INSERT_ADDON(3082,enhance_mp_addon_2);
	INSERT_ADDON(3083,enhance_attack_scale_addon_2arg);
	INSERT_ADDON(3084,enhance_defense_addon_2_2arg);
	INSERT_ADDON(3085,enhance_all_resistance_addon_2arg);
	INSERT_ADDON(3086,enhance_damage_reduce_addon_2arg);
	INSERT_ADDON(3087,enhance_all_magic_damage_reduce_addon_2arg);
	INSERT_ADDON(3088,reduce_cast_time_addon_2arg);
	INSERT_ADDON(3089, SET_ADDON_MACRO(3,enhance_all_magic_damage_reduce_addon));
	INSERT_ADDON(3090, SET_ADDON_MACRO(3,reduce_cast_time_addon));
	INSERT_ADDON(3091, SET_ADDON_MACRO(3,enhance_defend_degree));
	INSERT_ADDON(3092, SET_ADDON_MACRO(3,enhance_damage_reduce_addon));
	INSERT_ADDON(3093, SET_ADDON_MACRO(3,enhance_soulpower_addon));
	INSERT_ADDON(3094, SET_ADDON_MACRO(2,enhance_eng_addon_1arg));
	INSERT_ADDON(3095, SET_ADDON_MACRO(2,enhance_agi_addon_1arg));
	INSERT_ADDON(3096, SET_ADDON_MACRO(2,enhance_str_addon_1arg));
	INSERT_ADDON(3097, SET_ADDON_MACRO(2,enhance_eng_addon_1arg));
	INSERT_ADDON(3098, SET_ADDON_MACRO(2,enhance_agi_addon_1arg));
	INSERT_ADDON(3099, SET_ADDON_MACRO(2,enhance_str_addon_1arg));
	INSERT_ADDON(3100, SET_ADDON_MACRO(2,enhance_eng_addon_1arg));
	INSERT_ADDON(3101, SET_ADDON_MACRO(2,enhance_agi_addon_1arg));
	INSERT_ADDON(3102, SET_ADDON_MACRO(2,enhance_str_addon_1arg));
	INSERT_ADDON(3103, SET_ADDON_MACRO(4,enhance_damage_reduce_addon));
	INSERT_ADDON(3104, SET_ADDON_MACRO(4,enhance_damage_reduce_addon));
	INSERT_ADDON(3105, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(3106, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(3107, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(3108, SET_ADDON_MACRO(5,enhance_penetration));
	INSERT_ADDON(3109, enhance_penetration);
	INSERT_ADDON(3110, enhance_attack_degree);
	INSERT_ADDON(3111, reduce_cast_time_addon);
	INSERT_ADDON(3112, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(3113, SET_ADDON_MACRO(4,enhance_penetration));
	INSERT_ADDON(3114, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(3115, SET_ADDON_MACRO(6,enhance_hp_addon));
	INSERT_ADDON(3116, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(3117, SET_ADDON_MACRO(4,enhance_penetration));
	INSERT_ADDON(3118, SET_ADDON_MACRO(5,enhance_attack_speed_addon));
	INSERT_ADDON(3119, SET_ADDON_MACRO(6,enhance_hp_addon));
	INSERT_ADDON(3120, SET_ADDON_MACRO(3,enhance_resilience));
	INSERT_ADDON(3121, SET_ADDON_MACRO(4,enhance_penetration));
	INSERT_ADDON(3122, SET_ADDON_MACRO(5,reduce_cast_time_addon));
	INSERT_ADDON(3123, SET_ADDON_MACRO(6,enhance_hp_addon));
	INSERT_ADDON(3124, SET_ADDON_MACRO(6,enhance_resilience));
	INSERT_ADDON(3125, SET_ADDON_MACRO(6,enhance_resilience));
	INSERT_ADDON(3126, SET_ADDON_MACRO(6,enhance_resilience));
	INSERT_ADDON(3127, SET_ADDON_MACRO(8,enhance_penetration));
	INSERT_ADDON(3128, SET_ADDON_MACRO(8,enhance_penetration));
	INSERT_ADDON(3129, SET_ADDON_MACRO(8,enhance_penetration));
	INSERT_ADDON(3130, enhance_penetration);
	INSERT_ADDON(3131, enhance_penetration);
	INSERT_ADDON(3132, enhance_penetration);
	INSERT_ADDON(3133, TEMPORARY_ADDON_MACRO(enhance_mount_speed_addon));
	INSERT_ADDON(3134, TEMPORARY_ADDON_MACRO(enhance_mount_speed_addon));
	INSERT_ADDON(3135, TEMPORARY_ADDON_MACRO(enhance_mount_speed_addon));

	return true;
}

