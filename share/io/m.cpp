
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <malloc.h>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#include "mppc.h"
#include "octets.h"

using namespace GNET;

void dump(const Octets& o)
{
	int   size = o.size();
	const unsigned char *p = (const unsigned char *)o.begin();

	for(int i = 0; i < size; i++)
		fprintf(stderr, "%02x ", *p++);
	fprintf(stderr, "\n");
}

/*
void TestZLib( Octets & rand_data, bool btesttime )
{
	Octets os_src = rand_data;
	Octets os_com;

	os_com.reserve( compressBound(os_src.size()) );
	size_t len_com = os_com.capacity();
	if ( Z_OK == compress2((Bytef*)os_com.begin(),(uLongf*)&len_com,
						(Bytef*)os_src.begin(),os_src.size(), 1)
		&& len_com<os_src.size() )
	{
		os_com.resize(len_com);

		printf( "zlib compress ok. src size=%u, comp size=%u\n", os_src.size(), os_com.size() );

		size_t len_src = os_src.size();
		if (Z_OK == uncompress((Bytef*)os_src.begin(),(uLongf*)&len_src,
							(const Bytef*)os_com.begin(),os_com.size()))
		{
			if(!btesttime)
			{
				os_src.resize(len_src);
				printf( "zlib decompress ok. src size=%u, comp size=%u\n", os_src.size(), os_com.size() );
				if( os_src != rand_data )
					printf( "\tsrc and decompressed data is not equal.\n" );
			}
		}
		else
		{
			printf( "zlib decompress error, src size=%u, comp size=%u\n", os_src.size(), len_com );
		}
	}
	else
	{
		printf( "zlib compress error, src size=%u, comp size=%u\n", os_src.size(), len_com );
	}
}
*/


void TestMPPC( Octets & rand_data, bool btesttime )
{
	Octets os_src = rand_data;
	Octets os_com;

	os_com.reserve( mppc::compressBound(os_src.size()) );

	size_t len_com = os_com.capacity();

	if ( 0 == mppc::compress((unsigned char*)os_com.begin(),(int*)&len_com,
							(const unsigned char*)os_src.begin(),os_src.size())
		&& len_com<os_src.size() )
	{
		os_com.resize(len_com);

		printf( "mppc compress ok. src size=%u, comp size=%d\n", os_src.size(), os_com.size() );

		os_src.reserve(os_src.size()+4);
		size_t len_src = os_src.capacity();
		if (0 == mppc::uncompress((unsigned char*)os_src.begin(),(int*)&len_src,
							(const unsigned char*)os_com.begin(),os_com.size()))
		{
			if(!btesttime)
			{
				os_src.resize(len_src);
				printf( "mppc decompress ok. src size=%u, comp size=%d\n", os_src.size(), os_com.size() );
				if( os_src != rand_data )
				{
					printf( "\tsrc and decompressed data is not equal.\n" );
					dump(os_src);
					printf( "\n");
					dump(rand_data);
					abort();
				}
			}
		}
		else
		{
			printf( "mppc decompress error, src size=%u, comp size=%u\n", os_src.size(), len_com );
			abort();
		}
	}
	else
	{
		printf( "mppc compress error, src size=%u, comp size=%u\n", os_src.size(), len_com );
		abort();
	}
}

int xmain(int argc, char *argv[])
{
	Octets buf;
	buf.resize(8191);
	unsigned char *p = (unsigned char *)buf.begin();
	unsigned char *h = p;
	int  size = buf.size() / 4;


	int n = atoi(argv[1]);
	for (int i = 0; i < n ; i++)
	{
		for ( int i = 0; i < size; i++ ) { *(int *)p = rand() & 0xffff ; p += 4; } p = h;
		//memset(h, 1, 8191);
		TestMPPC(buf, false);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	malloc_stats();
	xmain(argc, argv);
	malloc_stats();
	return 0;
}
