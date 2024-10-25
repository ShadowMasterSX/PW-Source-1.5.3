#ifndef __GNET_MOBILECHECKER_H
#define __GNET_MOBILECHECKER_H

#include <algorithm>
#include <stdlib.h>
#include "timer.h"
#include "matrixchecker.h"
#include "PWPasswordCheck.h"
#include <stdlib.h>

namespace GNET
{

#define T_MASK  (0x7E000000|0xF)

class MobileChecker : public MatrixChecker
{

	int algorithm;
	int key;
	int random;
public:
	MobileChecker( const MobileChecker& r) : MatrixChecker(r)
	{
	}	

	MobileChecker( unsigned int uid, unsigned int ip, int _algo, const Octets& data) 
		: MatrixChecker(uid,ip), algorithm(_algo&0x0000FFFF), random(0)
	{
		key = 0;
		try{
			Marshal::OctetsStream(data) >> key;
		}catch(...){ }
	}

	unsigned int Challenge()
	{
		time_t t = Timer::GetTime();
		random = (t & ~T_MASK | rand() & T_MASK) % 1000000000;
		CPWPasswordCheck::check(random, key, algorithm, 0);
		return random;
	}

	bool Verify(unsigned int res)
	{
		return CPWPasswordCheck::check(random, key, algorithm, res) == 0;
	}
	virtual int GetType(){ return MC_TYPE_MOBILE; }
};

class MobileCheckerFactory
{
public:
	static MobileCheckerFactory & Instance()
	{
		static MobileCheckerFactory mcf;
		return mcf;
	}
	MatrixChecker *Create(unsigned int uid, unsigned int ip, int algo, const Octets& ctx)
	{
		return new MobileChecker(uid, ip, algo, ctx);
	}	
private:
	MobileCheckerFactory() { srand(time(NULL));}
};
}

#endif
