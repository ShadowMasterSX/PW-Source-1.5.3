#ifndef __GETHOSTIP_H
#define __GETHOSTIP_H

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>

#include <sys/ioctl.h> 
#include <sys/socket.h> 
#include <net/if.h> 
namespace GNET
{
	
const char* GetHostIP()
{
	static char ip[16];	
	int s;
	if ((s=socket(AF_INET,SOCK_DGRAM,0))==-1) return NULL;
	
	struct ifreq request;
	struct ifconf ifc;
	ifc.ifc_len = sizeof request; 
	ifc.ifc_buf = (caddr_t)&request;
   	
	if (!ioctl (s, SIOCGIFCONF, (char *) &ifc)) 
	{ 
		if (0==ifc.ifc_len / sizeof (struct ifreq)) 
		{
			close(s);
			return NULL;
		}
		if (!(ioctl (s, SIOCGIFADDR, (char*) &request)))
		{
			sprintf(ip,"%s",inet_ntoa(((struct sockaddr_in*)(&request.ifr_addr))->sin_addr));
			close(s);
			return ip;
		}
	}
	close(s);
	return NULL;
}

};
#endif
