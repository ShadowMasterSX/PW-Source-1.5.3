
#include <signal.h>

#include <iostream>
#include <string.h>
using std::cout;
using std::endl;

int main( int argc, char ** argv )
{
	if( argc != 2 )
	{
		cout << "usage: " << argv[0] << " -start/-stop/-restart" << endl;
		return 0;
	}

	if( 0 == strcmp(argv[1],"-start") || 0 == strcmp(argv[1],"start") )
	{
		cout << "start the program!" << endl;
		system( "./thread" );
	}
	else if( 0 == strcmp(argv[1],"-stop") || 0 == strcmp(argv[1],"stop") )
	{
		cout << "terminate the program!" << endl;
		system( "killall -SIGUSR1 thread" );
	}
	else if( 0 == strcmp(argv[1],"-restart") || 0 == strcmp(argv[1],"restart") )
	{
		cout << "restart the program!" << endl;
		system( "killall -SIGKILL thread" );
		system( "./thread" );
	}

	return 0;
}


