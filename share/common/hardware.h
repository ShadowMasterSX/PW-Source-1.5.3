#ifndef __HARDWARE_H
#define __HARDWARE_H

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>

#include <vector>
#include <map>

namespace GNET
{

class IFConfig
{
	std::vector<ifreq> ifreqs;
	IFConfig()
	{
		int s = socket( AF_INET, SOCK_DGRAM, 0 );
		struct ifconf ifc;
		for ( int i = 8; ; i <<= 1 )
		{
			ifreqs.resize(i);
			for ( std::vector<ifreq>::iterator it = ifreqs.begin(), ie = ifreqs.end(); it != ie; ++it )
				(*it).ifr_name[0] = 0;

			ifc.ifc_len = i * sizeof(struct ifreq);
			ifc.ifc_buf = (char *)&ifreqs[0];
			if ( ioctl( s, SIOCGIFCONF, &ifc ) == -1 )
			{
				ifreqs.clear();
				break;
			}
			if ( ifreqs[i - 1].ifr_name[0] == 0 )
			{
				std::vector<ifreq>::reverse_iterator it = ifreqs.rbegin(), ie = ifreqs.rend();
				for ( ; it != ie && (*it).ifr_name[0] == 0; ++it );
				ifreqs.resize( ie - it );
				break;
			}
		}
	}
public:
	static const IFConfig *GetInstance()
	{
		static IFConfig instance;
		return &instance;
	}
	
	size_t size() const { return ifreqs.size(); }
	const std::map<const char *, struct in_addr> if2ipv4() const
	{
		std::map<const char *, struct in_addr> map;
		for ( std::vector<ifreq>::const_iterator it = ifreqs.begin(), ie = ifreqs.end(); it != ie; ++it )
		{
			struct sockaddr_in *addr = (struct sockaddr_in *)&(*it).ifr_addr;
			map.insert( std::make_pair( (*it).ifr_name, addr->sin_addr ) );
		}
		return map;
	}
	size_t if2ipv4(std::vector<struct in_addr>& iplist) const
	{
		for ( std::vector<ifreq>::const_iterator it = ifreqs.begin(), ie = ifreqs.end(); it != ie; ++it )
		{
			struct sockaddr_in *addr = (struct sockaddr_in *)&(*it).ifr_addr;
			iplist.push_back(addr->sin_addr);
		}
		return iplist.size();
	}
};

}

#endif
