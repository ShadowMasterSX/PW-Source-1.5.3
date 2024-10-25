#ifndef __COMPRESS_H
#define __COMPRESS_H

#ifdef USE_ZLIB
#include "zlib.h"
#endif

#include "octets.h"
#include "marshal.h"
#include "mppc.h"

namespace GNET
{

static void Compress( Octets & os_src, Octets & os_com )
{
	unsigned int  len_src = os_src.size();

#ifdef USE_ZLIB
	//uLong len_com =  compressBound(len_src);
	unsigned int len_com =  compressBound(len_src);
	os_com.reserve(len_com);
	bool success = ( Z_OK == compress((Bytef*)os_com.begin(),&len_com,(Bytef*)os_src.begin(),len_src)
		&& len_com<len_src );
#else
	unsigned int len_com = mppc::compressBound(len_src);
	os_com.reserve(len_com);
	bool success;
	if( len_src <= 8192 )
	{
		success = ( 0 == mppc::compress((unsigned char*)os_com.begin(),(int*)&len_com,
								(const unsigned char*)os_src.begin(),len_src)
				&& len_com<len_src );
	}
	else
	{
		success = ( 0 == mppc::compress2((unsigned char*)os_com.begin(),(int*)&len_com,
								(const unsigned char*)os_src.begin(),len_src)
			&& len_com<len_src );
	}
#endif
	if( success )
	{
		Marshal::OctetsStream	os;
		os.reserve( 2 * sizeof(int) + len_com + os.size() );
		os << CompactUINT(len_src) << CompactUINT(len_com);
		os.push_byte( (const char*)os_com.begin(), len_com );
		os_com.swap( os );
	}
	else
	{
		Marshal::OctetsStream	os;
		os.reserve( 2 * sizeof(int) + len_src + os.size() );
		os << CompactUINT(len_src) << CompactUINT(len_src);
		os.push_byte( (const char*)os_src.begin(), len_src );
		os_com.swap( os );
	}
}

static void Uncompress( Octets & os_com, Octets & os_src )
{
	Marshal::OctetsStream os;
	os_com.swap( os );
	unsigned int  len_src, len_com;
	os >> CompactUINT(len_src) >> Marshal::Begin >> CompactUINT(len_com);

	if( len_com < len_src )
	{
		os >> Marshal::Rollback;
		os_com.reserve(len_com+8);
		os >> os_com;
		((Octets&)os_src).reserve( len_src );
#ifdef USE_ZLIB
		bool success = (Z_OK == uncompress((Bytef*)os_src.begin(),(uLongf*)&len_src,
							(const Bytef*)os_com.begin(),os_com.size()));
#else
		bool success = ( (len_src<=8192 && 0 == mppc::uncompress((unsigned char*)os_src.begin(),(int*)&len_src,
						(const unsigned char*)os_com.begin(),os_com.size()))
			|| (len_src>8192 && 0 == mppc::uncompress2((unsigned char*)os_src.begin(),(int*)&len_src,
								(const unsigned char*)os_com.begin(),os_com.size())) );
#endif
		os_src.resize( len_src );
		if( !success )
			os_src.swap( os_com );
		os_com.swap( os );
	}
	else
	{
		os >> Marshal::Rollback;
		os >> os_src;
		os_com.swap( os );
	}
}


}

#endif
