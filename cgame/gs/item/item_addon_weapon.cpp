//增强武器本体的属性 

class enhance_weapon_damage_addon: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_WEAPON_ESSENCE);
		ASSERT(arg_num == 2);
		if(datatype != (int)DT_WEAPON_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		weapon_essence * ess = (weapon_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int tmp = data.arg[0];
		ess->damage_low += tmp;
		ess->damage_high += tmp;
		return 0;
	}
};

class enhance_weapon_max_damage_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_WEAPON_ESSENCE);
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		weapon_essence * ess = (weapon_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int tmp = data.arg[0];
		ess->damage_high += tmp;
		return 0;
	}
};

class enhance_weapon_speed_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_WEAPON_ESSENCE);
		if(datatype != (int)DT_WEAPON_ESSENCE) return -1;
		float speed = *(float*)&(data.arg[0]);
		ASSERT(speed >= 0 && speed <= 1);
		data.arg[0] = (int)(speed * 20);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		weapon_essence * ess = (weapon_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int tmp = data.arg[0];
		ess->attack_speed -= tmp;
		return 0;
	}
};

class enhance_weapon_magic_addon: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_WEAPON_ESSENCE);
		if(datatype != (int)DT_WEAPON_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		weapon_essence * ess = (weapon_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int tmp = data.arg[0];
		ess->magic_damage_low += tmp;
		ess->magic_damage_high += tmp;
		return 0;
	}
};

class enhance_weapon_max_magic_addon : public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_WEAPON_ESSENCE);
		if(datatype != (int)DT_WEAPON_ESSENCE) return -1;
		ASSERT(data.arg[0] <= data.arg[1] && data.arg[0] > 0);
		data.arg[0] = abase::RandNormal(data.arg[0] , data.arg[1]);
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		weapon_essence * ess = (weapon_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		int tmp = data.arg[0];
		ess->magic_damage_high += tmp;
		return 0;
	}
};

class enhance_weapon_attack_range: public essence_addon
{
public:
	virtual int GenerateParam(int datatype, addon_data & data,int arg_num)
	{
		ASSERT(datatype == (int)DT_WEAPON_ESSENCE);
		if(datatype != (int)DT_WEAPON_ESSENCE) return -1;
		return 1;
	}

	virtual int ApplyAtGeneration(const addon_data & data, void * essence,size_t ess_size,prerequisition * require)
	{
		ASSERT(data.arg[0] > 0);
		weapon_essence * ess = (weapon_essence*)essence;
		ASSERT(ess_size == sizeof(*ess));
		float p = *(float*)&data.arg[0];
		ASSERT(p >= 0 && p < 10);
		ess->attack_range += p;
		return 0;
	}
};

