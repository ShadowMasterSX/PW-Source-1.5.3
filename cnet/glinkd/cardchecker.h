#ifndef __GNET_CARDCHECKER_H
#define __GNET_CARDCHECKER_H

#include <algorithm>
#include <stdlib.h>
#include "matrixchecker.h"

namespace GNET
{

class CardChecker : public MatrixChecker
{
#define IPMASK0 0x02000000
#define IPMASK1 0x00800000
#define IPMASK2 0x00010000
	unsigned int choose[3];
	unsigned int response;
public:
	CardChecker() { }
	CardChecker( const CardChecker& r) : MatrixChecker(r), response(r.response)
	{
		memcpy(choose, r.choose, sizeof(r.choose));
	}	

	CardChecker( unsigned int uid, unsigned int ip, unsigned char ctx[80] ) : MatrixChecker(uid,ip)
	{
		unsigned int mask = 0;
		unsigned int idx[80];
		unsigned int status = (uid >> 4) * (ctx[0]+ctx[1]*100+ctx[2]*10000+ctx[3]*1000000+ctx[4]*100000000);
		for ( int i = 0; i < 80; i++ ) idx[i] = i;
		for ( int i = 72; i < 80; i++, status = status * 1103515245 + 12345 ) std::swap( idx[i], idx[ status % 80 ] );
		if ( ip & IPMASK0 ) mask |= 1;
		if ( ip & IPMASK1 ) mask |= 2;
		if ( ip & IPMASK2 ) mask |= 4;
		choose[0] = idx[mask + 72];
		choose[1] = idx[rand() % 72];
	        unsigned int r = rand() % 72;
		choose[2] = idx[r];
		if ( choose[1] == choose[2]) 
			choose[2] = idx[(r+36)%72];
		mask = rand();
		std::swap ( choose[0], choose[mask % 3] );
		mask>>= 2;
		std::swap ( choose[1], choose[mask % 3] );
		mask>>= 2;
		std::swap ( choose[2], choose[mask % 3] );
		response = ctx[choose[0]]*10000 +  ctx[choose[1]]*100 + ctx[choose[2]];
	}

	unsigned int Challenge()
	{
		return choose[0] + choose[1] * 80 + choose[2] * 6400;
	}

	bool Verify( unsigned int res)
	{
		return response==res;
	}

	static void DecodePosition( unsigned int pos, unsigned int x[3], unsigned int y[3] )
	{
		unsigned int idx0, idx1, idx2;
		idx0 = pos % 80;
		pos /= 80;
		idx1 = pos % 80;
		pos /= 80;
		idx2 = pos % 80;
		x[0] = idx0 & 7;
		x[1] = idx1 & 7;
		x[2] = idx2 & 7;
		y[0] = idx0 >> 3;
		y[1] = idx1 >> 3;
		y[2] = idx2 >> 3;
	}
	virtual int GetType(){ return MC_TYPE_CARD; }
};

}

#endif
