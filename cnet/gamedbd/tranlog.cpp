#include "tranlog.h"
namespace GNET
{
	unsigned int LogKey::zid;
	unsigned int LogKey::max_serial;
}
#if 0
int main()
{
	srand(time(NULL));
	LogKey::Init(10);
	MyTransLog log("/tmp/logs");
	MyTransLog::Map::iterator it = log.Alloc();
	(int &)(*it).second = rand();
	(*it).second.SetDirty();
	MyTransLog::Map &map = log.GetMap();
	for ( MyTransLog::Map::iterator it = map.begin(), ie = map.end(); it != ie; ++it )
	{
		printf ("%d %d\n", (*it).first.GetSerial(), (int&)(*it).second );
	}
	log.CheckPoint();
	return 0;
}
#endif
