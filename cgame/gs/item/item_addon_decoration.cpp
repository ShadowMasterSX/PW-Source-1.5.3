//增强饰品属性的操作
template <size_t OFFSET>
class IA_ED_ESS : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data, int arg_num)
	{
		ASSERT(arg_num == 2);
		ASSERT(datatype == (int)DT_DECORATION_ESSENCE);
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		decoration_essence * ess = (decoration_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		ASSERT(ess_size >= OFFSET + 4);
		*(int*)((char*)essence + OFFSET) += data.arg[0];
		return 0;
	}
};

class item_decoration_specific_damage_addon: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(arg_num == 2);
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		ess->damage += data.arg[0];
		ess->defense -= data.arg[1];
		return 0;
	}
};

class item_decoration_specific_magic_damage_addon: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(arg_num == 2);
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		ess->magic_damage += data.arg[0];
		int tmp = data.arg[1];
		ess->resistance[0] -= tmp;
		ess->resistance[1] -= tmp;
		ess->resistance[2] -= tmp;
		ess->resistance[3] -= tmp;
		ess->resistance[4] -= tmp;
		return 0;
	}
};

template<size_t ENHANCE_MAGIC, size_t REDUCE_MAGIC>
class item_decoration_magic_resistance_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(arg_num == 2);
		return 2;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		ess->resistance[ENHANCE_MAGIC] += data.arg[0];
		ess->resistance[REDUCE_MAGIC] -= data.arg[1];
		return 0;
	}
};


class item_decoration_scale_enhance_magic_damage: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(arg_num == 1);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		float  p = *(float*)&data.arg[0];
		ess->magic_damage =(int)(ess->magic_damage * (1.f+p));
		return 0;
	}
};

class item_decoration_scale_enhance_damage: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(arg_num == 1);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		float  p = *(float*)&data.arg[0];
		ess->magic_damage =(int)(ess->magic_damage * (1.f+p));
		return 0;
	}
};

template <size_t OFFSET>
class item_decoration_enchance_resistance : public essence_addon
{
	virtual int GenerateParam(int datatype, addon_data & data, int arg_num)
	{
		ASSERT(arg_num == 2);
		ASSERT(datatype == (int)DT_DECORATION_ESSENCE);
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		ASSERT(OFFSET < MAGIC_CLASS);
		if(OFFSET >=MAGIC_CLASS) return -1;
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		ess->resistance[OFFSET] += data.arg[0];
		return 0;
	}
};

class item_decoration_enhance_all_resistance: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data, int arg_num)
	{
		ASSERT(arg_num == 2);
		ASSERT(datatype == (int)DT_DECORATION_ESSENCE);
		if(datatype != (int)DT_DECORATION_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		decoration_essence * ess = (decoration_essence*)essence;
		int tmp = data.arg[0];
		ess->resistance[0] += tmp;
		ess->resistance[1] += tmp;
		ess->resistance[2] += tmp;
		ess->resistance[3] += tmp;
		ess->resistance[4] += tmp;
		return 0;
	}
};

