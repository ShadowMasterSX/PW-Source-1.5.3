/*
 * FILE: ARandomGen.h
 *
 * DESCRIPTION: 
 *
 * CREATED BY: Duyuxin, 2004/7/31
 *
 * HISTORY: 
 *
 * Copyright (c) 2004 Archosaur Studio, All Rights Reserved.
 */

#ifndef _ARANDOMGEN_H_
#define _ARANDOMGEN_H_

#include "spinlock.h"
///////////////////////////////////////////////////////////////////////////
//	
//	Define and Macro
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Types and Global variables
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Declare of Global functions
//	
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//	
//	Class ARandomGen
//	
///////////////////////////////////////////////////////////////////////////

class ARandomGen
{
public:		//	Types

public:		//	Constructor and Destructor

	ARandomGen();
	ARandomGen(unsigned int seed);

public:		//	Attributes

public:		//	Operations

	//	Initialize random number generator
	void Init(unsigned int dwSeed) { RandomInitialize((int)(dwSeed % 31328), (int)(dwSeed % 30081)); }

	//	Generate a double random number
	double RandomUniform();
	//	Generate a double random number base on Gaussian formula
	double RandomGaussian(double mean, double stddev);
	//	Generate a integer random number
	//	Return random integer within a range, lower -> upper INCLUSIVE
	int RandomInt(int lower, int upper)
	{
		return ((int)(RandomUniform() * (upper - lower + 1)) + lower);
	}
	//	Generate a float random number
	//	Return random float within a range, lower -> upper
	float RandomFloat(float lower, float upper)
	{
		return (float)((upper - lower) * RandomUniform() + lower);
	}

protected:	//	Attributes

	double	u[97], c, cd, cm;
	int		m_i97, m_j97;
	int		m_bTest;

protected:	//	Operations

	//	Initialize random number generator
	void RandomInitialize(int ij, int kl);
};

namespace abase
{
#ifdef  __THREAD_SPEC_RAND__
	ARandomGen & GetRandomGenInstance();
	inline int RandT()
	{
		return (int)&GetRandomGenInstance();
	}
	inline int Rand(int lower, int upper)
	{	
		return 	GetRandomGenInstance().RandomInt(lower,upper);
	}
	inline float Rand(float lower, float upper)
	{	
		return GetRandomGenInstance().RandomFloat(lower,upper);
	}
	inline double RandGaussian(double mean, double stddev)
	{
		return GetRandomGenInstance().RandomGaussian(mean,stddev);
	}

	inline int RandNormal(int lower, int upper)
	{
		ARandomGen & ar = GetRandomGenInstance();
		double p = ar.RandomUniform() + ar.RandomUniform();
		p *= 0.5;
		return ((int)(p * (upper - lower + 1)) + lower);
	}

	inline float RandNormal(float lower, float upper)
	{
		ARandomGen & ar = GetRandomGenInstance();
		double p = ar.RandomUniform() + ar.RandomUniform();
		return (p * 0.5) *(upper - lower) + lower;
	}

	inline double RandUniform()
	{
		return GetRandomGenInstance().RandomUniform();
	}
	inline int RandSelect(const float * option, int size) 
	{ 
		float p = GetRandomGenInstance().RandomUniform();
		for(int i = 0; i < size; i ++,option ++)
		{
			if(p <= *option) return i;
			p -= *option;
		}
	//	ASSERT(p < 1e-6);  // 可能出现 ddr 141226
		return 0;
	}
	inline int RandSelect(const void * prob, int stride, int num)
	{
		ASSERT(prob && stride >= (int)sizeof(float) && num > 0);
		const char * option = (const char*)prob;
		float p = GetRandomGenInstance().RandomUniform();
		for(int i = 0; i < num; i ++,option += stride)
		{
			float tmp = *(float*)option; ASSERT(tmp >= 0 && tmp <=1.f);
			if(p <= tmp) return i;
			p -= tmp; ASSERT(p >= 0.f);
		}
	//	ASSERT(p < 1e-6);  // 可能出现 ddr 141226
		return 0;
	}
#else
	extern ARandomGen __global_RandomGen;
	extern int 	  __global_RandomLock;


	inline int Rand(int lower, int upper)
	{	
		spin_autolock keeper(__global_RandomLock);
		return 	__global_RandomGen.RandomInt(lower,upper);
	}
	inline float Rand(float lower, float upper)
	{	
		spin_autolock keeper(__global_RandomLock);
		return __global_RandomGen.RandomFloat(lower,upper);
	}
	inline double RandGaussian(double mean, double stddev)
	{
		spin_autolock keeper(__global_RandomLock);
		return __global_RandomGen.RandomGaussian(mean,stddev);
	}

	inline int RandNormal(int lower, int upper)
	{
		spin_autolock keeper(__global_RandomLock);
		double p = __global_RandomGen.RandomUniform() + __global_RandomGen.RandomUniform();
		p *= 0.5;
		return ((int)(p * (upper - lower + 1)) + lower);
	}

	inline float RandNormal(float lower, float upper)
	{
		spin_autolock keeper(__global_RandomLock);
		double p = __global_RandomGen.RandomUniform() + __global_RandomGen.RandomUniform();
		return (p * 0.5) *(upper - lower) + lower;
	}

	inline double RandUniform()
	{
		spin_autolock keeper(__global_RandomLock);
		return __global_RandomGen.RandomUniform();
	}
	inline int RandSelect(const float * option, int size) 
	{ 
		spin_autolock keeper(__global_RandomLock);
		float p = __global_RandomGen.RandomUniform();
		for(int i = 0; i < size; i ++,option ++)
		{
			if(p <= *option) return i;
			p -= *option;
		}
	//	ASSERT(p < 1e-6);  // 可能出现 ddr 141226
		return 0;
	}
	inline int RandSelect(const void * prob, int stride, int num)
	{
		ASSERT(prob && stride >= (int)sizeof(float) && num > 0);
		spin_autolock keeper(__global_RandomLock);
		const char * option = (const char*)prob;
		float p = __global_RandomGen.RandomUniform();
		for(int i = 0; i < num; i ++,option += stride)
		{
			float tmp = *(float*)option; ASSERT(tmp >= 0 && tmp <=1.f);
			if(p <= tmp) return i;
			p -= tmp; ASSERT(p >= 0.f);
		}
	//	ASSERT(p < 1e-6);  // 可能出现 ddr 141226
		return 0;
	}
#endif
	
};

///////////////////////////////////////////////////////////////////////////
//	
//	Inline functions
//	
///////////////////////////////////////////////////////////////////////////

#endif	//	_ARANDOMGEN_H_
