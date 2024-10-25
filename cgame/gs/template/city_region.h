#ifndef __ONLINE_GAME_GS_CITY_REGION_H__
#define __ONLINE_GAME_GS_CITY_REGION_H__

#include <common/types.h>

class CELRegion;
class grid;
namespace  city_region {
	bool InitRegionData(const char * precinct_path,const char * region_path);
	bool GetCityPos(float x, float z, A3DVECTOR & pos,int & world_tag);
	int GetDomainID(float x, float z);
	bool IsInPKProtectDomain(float x, float z);
	bool IsInSanctuary(float x, float z);
	bool GetRegionTransport(const A3DVECTOR & cur_pos, int tag, int ridx,  A3DVECTOR & target_pos, int & target_tag);
	void GetRegionTime(int &rtime, int &ptime);
	bool QueryTransportExist(int source_tag);
    CELRegion* GetRegion(float x, float z);
	bool InitSliceOverlapRegion(grid& psl);
};

#endif

