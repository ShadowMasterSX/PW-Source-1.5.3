#ifndef __COMPRESS_H
#define __COMPRESS_H

#include "octets.h"
#include "mppc.h"

namespace GNET
{
	void CompressMPPC( Octets & os_src, Octets & os_com );
	void UncompressMPPC( Octets & os_com, Octets & os_src );

#ifdef HAS_ZLIB
	void CompressZLIB( Octets & os_src, Octets & os_com );
	void UncompressZLIB( Octets & os_com, Octets & os_src );
#endif

	inline void Compress( Octets & os_src, Octets & os_com )
	{
		CompressMPPC(os_src,os_com);
	}
	inline void Uncompress( Octets & os_com, Octets & os_src )
	{
		UncompressMPPC(os_com,os_src);
	}
	// 返回压缩后的数据长度，返回－1压缩失败
	inline int MPPCCompress( unsigned char* src, int src_len, Octets& dst)
	{
		int com_len = mppc::compressBound(src_len);
		dst.reserve(com_len);
		if( src_len <= 8192 )
		{
			if(mppc::compress((unsigned char*)dst.begin(),&com_len,src,src_len))
			{
				return -1;
			}
		}
		else
		{
			//需要分组
			if(mppc::compress2((unsigned char*)dst.begin(),&com_len,src,src_len))
				return -1;
		}
		dst.resize(com_len);
		return com_len;
	}

	// 返回解压后的数据长度，失败返回－1，解压字节数不会超过dst_len
	inline int MPPCDecompress( unsigned char* src, int src_len, unsigned char* dst, int dst_len)
	{
		if(dst_len<=8192)
		{
			if(mppc::uncompress(dst,&dst_len,src,src_len))
				return -1;
		}
		else
		{
			if(mppc::uncompress2(dst,&dst_len,src,src_len))
				return -1;
		}
		return dst_len;
	}
}

#endif
