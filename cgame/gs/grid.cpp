#include <stdlib.h>
#include <ASSERT.h>
#include "grid.h"
#include "config.h"
#include "template/city_region.h"

grid::grid():pTable(0),grid_region(0.f,0.f,0.f,0.f),
			base_region(0.f,0.f,0.f,0.f),
			local_region(0.f,0.f,0.f,0.f),
			slice_step(0.f),inv_step(0.f)
{
}

grid::~grid()
{
	if(pTable) abase::fast_allocator::raw_free(pTable);
}

const grid & 
grid::operator=(const grid & rhs)
{
	ASSERT(pTable == 0 && &rhs != this);
	ASSERT(rhs.pTable);

	grid_region 	= rhs.grid_region;
	base_region	= rhs.base_region;
	local_region	= rhs.local_region;
	inner_region	= rhs.inner_region;
	slice_step	= rhs.slice_step;
	inv_step	= rhs.inv_step;
	slice_count	= rhs.slice_count;
	reg_row		= rhs.reg_row;
	reg_column	= rhs.reg_column;

	int total = reg_row * reg_column;
	pTable = (slice *)abase::fast_allocator::raw_alloc(total*sizeof(slice));
	memcpy(pTable,rhs.pTable,total * sizeof(slice));

#ifndef NDEBUG
	for(size_t i = 0; i < (size_t)total; i ++)
	{
		ASSERT(pTable[i].spinlock == 0);
		ASSERT(pTable[i].player_list == 0);
		ASSERT(pTable[i].npc_list == 0);
		ASSERT(pTable[i].matter_list == 0);
	}
#endif
	return *this;

}

bool grid::Create(int row,int column,float step,float sx,float sz)
{
	ASSERT(pTable == NULL);
	grid_region.left = sx;
	grid_region.top = sz;
	grid_region.right = sx + column * step;
	grid_region.bottom = sz + row * step;
	inv_step = 1.f/step;
	slice_step = step;
	reg_row = row;
	reg_column = column;

	int total = row * column;
	pTable = (slice *)abase::fast_allocator::raw_alloc(total*sizeof(slice));
	int i;
	for(i = 0; i < row*column; i ++)
	{
		/*现在没有用了，因为grid的距离也不是一格
		if(i <column || i > total - column || (i % column == 0) || (i % column == column - 1))
		{
			pTable[i].flag |= slice::GRID_EDGE;
		}*/
		float left = (i % column) * step + sx;
		float top = (i / column) * step + sz;
		pTable[i].slice_range = rect(left,top,left + step,top + step);
		pTable[i].region_overlap = false;
	}
	slice_count = row*column;
	return true;
}

static bool spec_overlap(const rect & large, const rect & small)
{
	return large.IsIn(small.left,small.top) || 
		large.IsIn(small.left,small.bottom) ||
		large.IsIn(small.right,small.top) ||
		large.IsIn(small.right,small.bottom);
}

static bool complete_overlap(const rect & large, const rect & small)
{
	return large.IsIn(small.left,small.top) && 
		large.IsIn(small.left,small.bottom) &&
		large.IsIn(small.right,small.top) &&
		large.IsIn(small.right,small.bottom);
}

bool grid::SetRegion(const rect &local_rt, const rect & base_rt,float border_size)
{
	if(base_rt.Width() <=0 || base_rt.Height() <=0 || local_rt.Width() <=0 || local_rt.Height() <=0) return false;
	if(!grid_region.IsIn(base_rt)) return false;
	if(!local_rt.IsIn(base_rt)) return false;
	base_region = base_rt;
	local_region = local_rt;

//重新计算每个格子所处的地位
	rect ne_region = local_region;	//去除边界的区域
	ne_region.left += slice_step;
	ne_region.top += slice_step;
	ne_region.bottom -= slice_step;
	ne_region.right -= slice_step;

	rect in_region = local_region;	//内部区域 （与其他服务器一般不搭界的区域）这并不准确
	in_region.left += border_size + slice_step;
	in_region.top += border_size + slice_step;
	in_region.bottom -= border_size + slice_step;
	in_region.right -= border_size + slice_step;

	rect out_region = local_region;	//外部敏感区（超出这个区域的就不再与本服务器相关）
	out_region.left -= border_size;
	out_region.top -= border_size;
	out_region.bottom -= border_size;
	out_region.right -= border_size;

	int c1=0,c2=0,c3=0;
	for(int i = 0; i < slice_count; i ++)
	{
		rect rt = pTable[i].slice_range;
		ASSERT(spec_overlap(local_region,rt)?1:0 == local_region.IsOverlap(rt)?1:0);  
		if(!local_region.IsOverlap(rt))
		{
			//不相干区域
			pTable[i].flag |= slice::OUTSIDE;
			if(out_region.IsOverlap(rt))
			{
				pTable[i].flag |= slice::SENSITIVE;
			}
			continue; 
		}
		c1++;
		pTable[i].flag |= slice::INSIDE;
		if(in_region.IsIn(rt)) continue;	//在内部
		c2++;

		pTable[i].flag |= slice::BORDER;	
		if(ne_region.IsIn(rt)) continue;	//在边界处
		c3++;

		pTable[i].flag |= slice::EDGE;		//就在边上
	}
	return true;
}

void grid::InitRegionOverlap(rect& rt)
{
	if(!complete_overlap(grid_region,rt))//  region 超过 slice 覆盖范围 is20/is32/is43/is50
	{
		rt.left   = rt.left < grid_region.left ? grid_region.left : rt.left;
		rt.right  = rt.right > grid_region.right ? grid_region.right : rt.right;
		rt.top    = rt.top < grid_region.top ? grid_region.top : rt.top;
		rt.bottom = rt.bottom > grid_region.bottom ? grid_region.bottom: rt.bottom;
	}

	int lu_ofx = (int)((rt.left - grid_region.left) * inv_step);
	int lu_ofy = (int)((rt.top - grid_region.top) * inv_step);
	int rb_ofx = (int)((rt.right - grid_region.left) * inv_step);
	int rb_ofy = (int)((rt.bottom - grid_region.top) * inv_step); 
	for(int row = lu_ofy; row <= rb_ofy; ++row)
	{
		for(int col = lu_ofx; col <= rb_ofx; ++ col)
		{
			unsigned int offset = (unsigned int)(col + row * reg_column);
			ASSERT(offset < (unsigned int)slice_count);
			slice* pSlice = pTable + offset;
			pSlice->region_overlap = true;
		}
	}
}

slice * grid::Locate(float x,float z) const
{
	ASSERT(grid_region.IsIn(x,z));
	int ofx = (int)((x - grid_region.left) * inv_step);
	int ofz = (int)((z - grid_region.top) * inv_step);
	unsigned int offset  = (unsigned int)(ofx + ofz * reg_column);
	if(offset >= (unsigned int)slice_count) return NULL;
	return pTable + offset;
}

slice * grid::Locate(float x,float z,int &rx,int &rz) const
{
	ASSERT(grid_region.IsIn(x,z));
	int ofx = (int)((x - grid_region.left) * inv_step);
	int ofz = (int)((z - grid_region.top) * inv_step);
	rx = ofx;rz = ofz;
	unsigned int offset  = (unsigned int)(ofx + ofz * reg_column);
	if(offset >= (unsigned int)slice_count) return NULL;
	return pTable + offset;
}


