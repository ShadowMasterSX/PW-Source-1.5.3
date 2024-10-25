#ifndef __ONLINEGAME_GS_BREATH_CONTROL_H__
#define __ONLINEGAME_GS_BREATH_CONTROL_H__

#include <ASSERT.h>
#include <common/base_wrapper.h>

class gplayer_imp;
struct breath_ctrl
{
	int _breath;
	int _breath_capacity;
	short _breath_state;
	char _endless_breath;
	char _under_water;
	int _point_adjust;
	float _water_offset;
	
	enum 
	{
		BREATH_NORMAL,
		BREATH_UNDERWATER,
	};
	enum
	{
		DAMAGE_OUT_OF_BREATH = 15
	};

	enum 
	{
		BASE_BREATH_VALUE = 120
	};

public:
	breath_ctrl():_breath(0),_breath_capacity(BASE_BREATH_VALUE),_breath_state(BREATH_NORMAL),_endless_breath(0),_under_water(0),_point_adjust(0),_water_offset(0.0f)
	{
	}

	inline bool IsUnderWater(float offset)
	{
		return _under_water && _water_offset > offset;
	}

	inline void SetUnderWater(bool under_water, float offset)
	{
		_under_water = under_water;
		_water_offset = offset; 
	}

	bool IncBreath(int count)
	{
		ASSERT(count > 0);
		if(_breath_state != BREATH_UNDERWATER) return false;
		if(_breath == _breath_capacity) return false;
		if((_breath += count) > _breath_capacity) _breath = _breath_capacity;
		return true;
	}

	void EnhanceBreathCapacity(int count)
	{
		_breath_capacity += count;
	}
	
	void ImpairBreathCapacity(int count)
	{
		_breath_capacity -= count;
		if(_breath > _breath_capacity) _breath = _breath_capacity;
	}

	void EnableEndlessBreath(bool en)
	{
		_endless_breath = en?1:0;
	}

	void AdjustPointAdjust(int offset)
	{
		_point_adjust += offset;
	}

	void ChangeState(gplayer_imp * pImp , bool in_water);
	void OnHeartbeat(gplayer_imp * pImp);
	void Save(archive & ar);
	void Load(archive & ar);

	void Swap(breath_ctrl & rhs)
	{
		breath_ctrl p = rhs;
		rhs = *this;
		*this = p;
	}
};
#endif

