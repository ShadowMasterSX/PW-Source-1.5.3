#ifndef __GNET_SERVERLOAD_H
#define __GNET_SERVERLOAD_H

#include "localmacro.h"

namespace GNET
{

class ServerLoad
{
private:
	int server_limit;
	int server_count;

public:
	ServerLoad():server_limit(0), server_count(0) { }
	
	void SetLoad(int srv_limit, int srv_count)
	{
		server_limit = srv_limit;
		server_count = srv_count;
	}
	
	void ClearLoad()
	{
		server_limit = 0;
		server_count = 0;
	}

	int CheckLimit() const 
	{
		if(server_count >= server_limit) return ERR_SERVEROVERLOAD;
		return ERR_SUCCESS; 
	}
};

};
#endif
