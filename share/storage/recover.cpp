
#include <vector>
#include <map>
#include <iostream>
#include <netinet/in.h>

#include "storage.h"
#include "conf.h"

using namespace std;
using namespace GNET;

int main( int argc, char **argv )
{
	// run backup
	if (argc < 3 && access(argv[1], R_OK) == -1)
	{
		std::cerr << "Usage: " << argv[0] << " configurefile backupdir" << std::endl;
		exit(-1);
	}

	GNET::Conf::GetInstance("io.conf");
	std::string srcdir = argv[2];
	std::cout << "Recovering " << srcdir << std::endl;

	// open db env
	StorageEnv::Open();
	// StorageEnv::recover( srcdir.c_str() );
 
	StorageEnv::Close();
	return (EXIT_SUCCESS);
}


