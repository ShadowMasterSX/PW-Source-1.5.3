#ifndef ANTI_ADDICTION_HPP_INCLUDE__
#define ANTI_ADDICTION_HPP_INCLUDE__

#include <time.h>
#include <string>

namespace Addiction
{
	enum OnlineState 
	{
		S_NORMAL,  
		S_LIGHT_PANISH,  
		S_WEIGHT_PANISH, 
	};

	class IAddictionData
	{
	public:
		virtual void GetTimeLeft(time_t * light_time, time_t * weight_time, int * play_time) = 0;
		virtual void Tick(time_t now, OnlineState *state) = 0;
		virtual bool Init(time_t now, const void *buf, int len) = 0;
		virtual void GetData(time_t now, void **ptr, int *size) = 0;

		virtual ~IAddictionData();
	};

	void SetAddictionParam(int light_time, int weight_time, int clear_time, int calc_mode);
	IAddictionData * CreateAddictionObj(int now);


}; //namespace Addiction

#endif //ANTI_ADDICTION_HPP_INCLUDE__
