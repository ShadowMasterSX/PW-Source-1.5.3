#include "city_region.h"
#include "el_precinct.h"
#include "el_region.h"
#include "../world.h"
#include "../worldmanager.h"

static CELPrecinctSet __gl_ps;
static CELRegionSet __gl_rs;

namespace  city_region {
bool InitRegionData(const char * precinct_path,const char * region_path)
{
	return __gl_ps.Load(precinct_path) && __gl_rs.Load(region_path);
}
void GetRegionTime(int &rtime, int &ptime)
{
	ptime = __gl_ps.GetTimeStamp();
	rtime = __gl_rs.GetTimeStamp();
}

bool GetCityPos(float x, float z, A3DVECTOR & pos,int & world_tag)
{
	CELPrecinct*  pPrecinct = __gl_ps. IsPointIn(x, z,world_manager::GetWorldTag());
	if(pPrecinct) 
	{
		const CELPrecinct::VECTOR3&  vec = pPrecinct->GetCityPos();
		pos.x = vec.x;
		pos.y = vec.y;
		pos.z = vec.z;
		world_tag = pPrecinct->GetDstInstanceID();
		return true;
	}
	else
	{
		return false;
	}
}

int GetDomainID(float x, float z)
{
	CELPrecinct*  pPrecinct = __gl_ps. IsPointIn(x, z,world_manager::GetWorldTag());
	if(pPrecinct) 
	{
		return pPrecinct->GetDomainID();
	}
	return -1;
}

bool IsInPKProtectDomain(float x, float z)
{
	CELPrecinct*  pPrecinct = __gl_ps. IsPointIn(x, z,world_manager::GetWorldTag());
	if(pPrecinct)
	{
		return pPrecinct->IsPKProtect();
	}
	return false;
}

bool IsInSanctuary(float x, float z)
{
	//CELRegion*  pRegion = __gl_rs.IsPointInSanctuary(x, z);
	CELRegion*  pRegion = __gl_rs.IsPointInRegion(x, z);
	if(pRegion) 
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool GetRegionTransport(const A3DVECTOR & cur_pos, int tag, int ridx,  A3DVECTOR & target_pos, int & target_tag)
{
	CELTransportBox*  pBox = __gl_rs.GetTransportBox(ridx);
	if(!pBox) return false;

	if(world_manager::GetWorldTag() != pBox->GetSrcInstanceID()) return false;

	if(!pBox->IsPointIn(cur_pos.x,cur_pos.y,cur_pos.z))
	{
		__PRINTF("副本传送区域不正确 %f,%f,%f\n",cur_pos.x,cur_pos.y,cur_pos.z);
		//区域不正确
		return false;
	} 
	
	if((target_tag = pBox->GetInstanceID()) != tag)
	{
		//目标不正确
		return false;
	}

	const CELTransportBox::VECTOR3 &t = pBox->GetTargetPos();
	target_pos.x = t.x;
	target_pos.y = t.y;
	target_pos.z = t.z;
	return true;
}

bool QueryTransportExist(int source_tag)
{
	size_t n = __gl_rs.GetTransportBoxNum();
	for(size_t i = 0; i < n ; i ++)
	{
		CELTransportBox*  pBox = __gl_rs.GetTransportBox(i);
		if(!pBox) 
		{
			ASSERT(false);
		}
		if(source_tag != pBox->GetSrcInstanceID()) return false;
	}
	return true;
}

CELRegion* GetRegion(float x, float z)
{
    return __gl_rs.IsPointInRegion(x, z);
}

bool InitSliceOverlapRegion(grid& psl)
{
	struct timeval tv_beg,tv_end;
	gettimeofday(&tv_beg,NULL);
	for(int i = 0; i < __gl_rs.GetRegionNum(); ++i)
	{
		CELRegion* prg = __gl_rs.GetRegion(i);		
		ASSERT(prg);
		rect rt;
		prg->GetRect(rt.left, rt.right, rt.top ,rt.bottom);
		psl.InitRegionOverlap(rt);
	}
	gettimeofday(&tv_end,NULL);
	int costtime = 1000000*(tv_end.tv_sec - tv_beg.tv_sec) + tv_end.tv_usec - tv_beg.tv_usec;
	__PRINTINFO("slice与region交叠运算耗时%d 微秒\n",costtime);
	return true;
}

};

