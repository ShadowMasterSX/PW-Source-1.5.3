#ifndef __MPPC_H
#define __MPPC_H

#include <sys/types.h>


namespace GNET
{

class mppc
{
public:
	static size_t compressBound( size_t sourcelen );

	static int compress( unsigned char *dest, int *destLen, const unsigned char *source, int sourceLen ); //压缩，成功时返回0. destLen为压缩后的长度
	static int uncompress( unsigned char *dest, int *destLen, const unsigned char *source, int sourceLen);//解压，成功时返回0

	static int compress2( unsigned char *dest, int *destLen, const unsigned char *source, int sourceLen );//压缩，会将输入按8192大小分片
	static int uncompress2( unsigned char *dest, int *destLen, const unsigned char *source, int sourceLen);//解压用compress2压缩出来的buffer
};

}

#endif
