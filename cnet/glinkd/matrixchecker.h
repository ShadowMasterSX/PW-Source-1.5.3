#ifndef __GNET_MATRIXCHECKER_H
#define __GNET_MATRIXCHECKER_H

namespace GNET
{

enum
{
	MC_TYPE_CARD 		= 1,
	MC_TYPE_MOBILE 		= 2,
	MC_TYPE_PHONETOKEN 	= 3,
};

class MatrixChecker
{
	unsigned int uid;
	unsigned int ip;
public:
	MatrixChecker() {}
	MatrixChecker( unsigned int _uid, unsigned int _ip ) : uid(_uid), ip(_ip) {}
	MatrixChecker( const MatrixChecker& r) : uid(r.uid), ip(r.ip) {} 
	virtual ~MatrixChecker() {}
	virtual unsigned int Challenge() { return 0; } 
	virtual bool Verify( unsigned int response) { return true; }
	virtual bool GetUsedElecNumber(Octets & num) { return false; }
	unsigned int GetUid() { return uid; }
	unsigned int GetIp()  { return ip; }
	virtual int GetType() = 0;
};

}

#endif

