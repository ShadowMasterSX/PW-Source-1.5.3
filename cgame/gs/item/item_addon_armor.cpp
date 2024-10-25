
//增强防具本体的属性(双参数)

template <size_t offset>
class IA_EA_ESS	 : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_ARMOR_ESSENCE);
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		armor_essence * ess = (armor_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		ASSERT(ess_size >= offset + 4);
		*(int*)((char*)essence + offset) += data.arg[0];
		return 0;
	}
};


class item_armor_enhance_all_resistance : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(arg_num == 2);
		ASSERT(datatype == (int)DT_ARMOR_ESSENCE);
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		armor_essence * ess = (armor_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int offset = data.arg[0];
		ess->resistance[0] += offset;
		ess->resistance[1] += offset;
		ess->resistance[2] += offset;
		ess->resistance[3] += offset;
		ess->resistance[4] += offset;
		return 0;
	}
};

template <size_t OFFSET>
class item_armor_enhance_resistance : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_ARMOR_ESSENCE);
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		ASSERT(OFFSET < MAGIC_CLASS);
		if(OFFSET >=MAGIC_CLASS) return -1;
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		armor_essence * ess = (armor_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int value = data.arg[0];
		ess->resistance[OFFSET] += value;
		return 0;
	}
};


template <size_t OFFSET>
class item_armor_scale_enhance_resistance : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(OFFSET < MAGIC_CLASS);
		ASSERT(datatype == (int)DT_ARMOR_ESSENCE);
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		float p0 = *(float*)&(data.arg[0]);
		float p1 = *(float*)&(data.arg[1]);
		p0 = abase::Rand(p0,p1);
		data.arg[0] = *(int*)&p0;
		if(OFFSET >=MAGIC_CLASS) return -1;
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		armor_essence * ess = (armor_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		float p = *(float*)&(data.arg[0]);
		if(fabs(p) > 10 ) return 0;
		float nr = ess->resistance[OFFSET] * (1.f + p);
		ess->resistance[OFFSET] = (int)nr;
		return 0;
	}
};

template <size_t ENHANCE_RESIST, size_t REDUCE_RESIST>
class item_armor_enhance_resistance_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(ENHANCE_RESIST < MAGIC_CLASS);
		ASSERT(REDUCE_RESIST < MAGIC_CLASS);
		ASSERT(datatype == (int)DT_ARMOR_ESSENCE);
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		float p0 = *(float*)&(data.arg[0]);
		float p1 = *(float*)&(data.arg[1]);
		ASSERT(p0 >=0 && p1 <=1.f);
		ASSERT(p1 >=0 && p1 <=1.f);
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		armor_essence * ess = (armor_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int value = ess->resistance[ENHANCE_RESIST];
		value = (int)(value * (1 + (*(float*)&(data.arg[0]))));
		ess->resistance[ENHANCE_RESIST] = value;

		value = ess->resistance[REDUCE_RESIST];
		value = (int)(value * (1 - (*(float*)&(data.arg[1]))));
		ess->resistance[REDUCE_RESIST] = value;
		return 0;
	}
};

template<size_t ENHANCE_MAGIC, size_t REDUCE_MAGIC>
class item_armor_enhance_resistance_addon_2 : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		ASSERT(arg_num == 3);
		data.arg[0] = abase::RandNormal(data.arg[0] - data.arg[2], data.arg[0] + data.arg[2]);
		data.arg[1] = abase::RandNormal(data.arg[1] - data.arg[2], data.arg[1] + data.arg[2]);
		ASSERT(data.arg[0] >= -10240 && data.arg[0] <= 10240); 
		ASSERT(data.arg[1] >= -10240 && data.arg[1] <= 10240); 
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		armor_essence * ess = (armor_essence*)essence;
		ess->resistance[ENHANCE_MAGIC] += data.arg[0];
		ess->resistance[REDUCE_MAGIC] -= data.arg[1];
		return 0;
	}
};

template<size_t ENHANCE_MAGIC, size_t REDUCE_MAGIC>
class item_armor_enhance_resistance_addon_3 : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		ASSERT(data.arg[0] >=0  && data.arg[0] <= 10240); 
		ASSERT(data.arg[1] >=0  && data.arg[1] <= 10240); 
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		armor_essence * ess = (armor_essence*)essence;
		ess->resistance[ENHANCE_MAGIC] += data.arg[0];
		ess->resistance[REDUCE_MAGIC] -= data.arg[1];
		return 0;
	}
};

class item_armor_specific_addon: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		ASSERT(arg_num == 2);
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		armor_essence * ess = (armor_essence*)essence;
		ess->defense += data.arg[0];
		return 0;
	}

	virtual int UpdateItem(const addon_data & data, equip_item * equip)
	{
		equip->_base_param.damage_low -= data.arg[1];
		equip->_base_param.damage_high -= data.arg[1];
		return addon_manager::ADDON_MASK_ESSENCE;
	}
};

template <size_t offset>
class IA_EA_ESS_SCALE : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_ARMOR_ESSENCE);
		if(datatype != (int)DT_ARMOR_ESSENCE) return -1;
		float p0 = *(float*)&(data.arg[0]);
		float p1 = *(float*)&(data.arg[1]);
		p0 = abase::Rand(p0,p1);
		data.arg[0] = *(int*)&p0;
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		armor_essence * ess = (armor_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		ASSERT(ess_size >= offset + 4);
		float p = 1.f + *(float*)&data.arg[0];
		int * value= (int*)((char*)essence + offset);
		*value = (int)((*value) * p);
		return 0;
	}
};

