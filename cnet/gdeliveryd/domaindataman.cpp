#include "domaindataman.h"
#include <cstdio>

static std::vector<DOMAIN_INFO_SERV>		domain_infos_server;
static std::vector<BATTLETIME_SERV>			battletime_server;
static int									battletime_max;
static std::vector<DOMAIN2_INFO_SERV>		domain2_infos_server;
static unsigned int							domain2_data_timestamp;

///////////////////////////////////////////////////////////////////////////////
// load data from a config file
// return	0 if succeed
//			-1 if failed.
///////////////////////////////////////////////////////////////////////////////
int domain_data_load()
{
	int	count;
	FILE * fpServer;

	fpServer = fopen("domain.sev", "rb");
	if( !fpServer )	return -1;
	fread(&count, 1, sizeof(int), fpServer);
	for(int i=0; i<count; i++)
	{
		DOMAIN_INFO_SERV domain_info;
		fread(&domain_info.id, 1, sizeof(int), fpServer);
		fread(&domain_info.type, 1, sizeof(int), fpServer);
		fread(&domain_info.reward, 1, sizeof(int), fpServer);
		int nNumNeighbours;
		fread(&nNumNeighbours, 1, sizeof(int), fpServer);
		for(int j=0; j<nNumNeighbours; j++)
		{
			int idNeighbour;
			fread(&idNeighbour, 1, sizeof(int), fpServer);
			domain_info.neighbours.push_back(idNeighbour);
		}
		domain_infos_server.push_back(domain_info);
	}
	// load time list
	fread(&count, 1, sizeof(int), fpServer);
	for(int i=0; i<count; i++)
	{
		BATTLETIME_SERV theTime;
		fread(&theTime, sizeof(BATTLETIME_SERV), 1, fpServer);
		battletime_server.push_back(theTime);
	}
	fread(&battletime_max, sizeof(int), 1, fpServer);

	fclose(fpServer);
	return 0;
}

int domain2_data_load(char domain2_type)
{
	FILE* fpServer = NULL;
	
	if(domain2_type == DOMAIN2_TYPE_CROSS) {
		fpServer = fopen("domain2_cross.sev", "rb");
	} else {
		fpServer = fopen("domain2.sev", "rb");
	}

	if( fpServer == NULL )	return -1;
	
	fread(&domain2_data_timestamp, 1, sizeof(unsigned int), fpServer);
	
	int	count = 0;
	fread(&count, 1, sizeof(int), fpServer);
	
	for(int i = 0; i < count; ++i) {
		DOMAIN2_INFO_SERV domain2_info;
		
		fread(&domain2_info.id, 1, sizeof(int), fpServer);
		fread(&domain2_info.point, 1, sizeof(int), fpServer);
		fread(&domain2_info.wartype, 1, sizeof(int), fpServer);
		fread(&domain2_info.country_id, 1, sizeof(int), fpServer);
		fread(&domain2_info.is_capital, 1, sizeof(int), fpServer);
		fread(&domain2_info.mappos, 1, sizeof(domain2_info.mappos), fpServer);	

		int nNumNeighbours = 0;
		fread(&nNumNeighbours, 1, sizeof(int), fpServer);
		
		for(int j = 0; j < nNumNeighbours; ++j) {
			int idNeighbour = -1;
			fread(&idNeighbour, 1, sizeof(idNeighbour), fpServer);
			domain2_info.neighbours.push_back(idNeighbour);

		}
		
		for(int j = 0; j < nNumNeighbours; ++j) {
			int time = 0;
			fread(&time, 1, sizeof(time), fpServer);
			domain2_info.time_neighbours.push_back(time);
		}
		
		domain2_infos_server.push_back(domain2_info);
	}

	fclose(fpServer);
	return 0;
}

// get the data of a domain by id
DOMAIN_INFO_SERV * domain_data_getbyid(int id)
{
	int n = domain_infos_server.size();
	for(int i=0; i<n; i++)
	{
		if( domain_infos_server[i].id == id )
			return &domain_infos_server[i];
	}

	return NULL;
}

// get the data of a domain by index
DOMAIN_INFO_SERV * domain_data_getbyindex(int index)
{
	int n = domain_infos_server.size();
	if( index >= n )
		return NULL;

	return &domain_infos_server[index];
}

// get the number of domain data
int domain_data_getcount()
{
	return domain_infos_server.size();
}

const std::vector<BATTLETIME_SERV>& getbattletimelist()
{
	return battletime_server;
}

int getbattletimemax()
{
	return battletime_max;
}

DOMAIN2_INFO_SERV* domain2_data_getbyid(int id)
{
	int n = domain2_infos_server.size();
	for(int i = 0; i < n; ++i) {
		if( domain2_infos_server[i].id == id ) return &domain2_infos_server[i];
	}

	return NULL;
}

DOMAIN2_INFO_SERV* domain2_data_getbyindex(int index)
{
	int n = domain2_infos_server.size();
	if( index >= n ) return NULL;

	return &domain2_infos_server[index];
}

DOMAIN2_INFO_SERV * domain2_get_capital_data(int country_id)
{
	int n = domain2_infos_server.size();
	for(int i = 0; i < n; ++i) {
		if( domain2_infos_server[i].is_capital && (domain2_infos_server[i].country_id == country_id) ) return &domain2_infos_server[i];
	}

	return NULL;
}

const std::vector<DOMAIN2_INFO_SERV>* get_domain2_infos()
{
	return &domain2_infos_server;
}

unsigned int get_domain2_data_timestamp()
{
	return domain2_data_timestamp;
}

