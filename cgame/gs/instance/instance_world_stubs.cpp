#include "instance_manager.h"

static instance_world_manager *iwm = NULL;

int InitWorld(const char * conf_file, const char * gmconf_file, const char * servername)
{
	iwm = new instance_world_manager();
	return iwm->Init(conf_file,gmconf_file,servername);
}
