#include <spinlock.h>
#include <math.h>
#include <common/types.h>
#include "object.h"
#include "player.h"
#include "slice.h"

#ifndef __ONLINEGAME_GS_GRID_H__
#define __ONLINEGAME_GS_GRID_H__

struct grid
{

	slice * pTable;		//所有分区组成的表
	rect  	grid_region;	//整个分区所管辖的区域
	rect  	base_region;	//基准区域，这个值没有使用,但是这个区域是负责的基准
	rect  	local_region;	//本地区域，超过了这个区域应该进行服务器转移
	rect  	inner_region;	//内部区域这个区域只有自己负责
	float 	slice_step;	//步长的大小
	float 	inv_step;	//步长的大小的倒数
	int 	slice_count;
	int	reg_row;
	int	reg_column;
	
public:
	grid();
	~grid();
	const grid & operator=(const grid & rhs);
	bool Create(int row,int column,float step,float startX,float startZ);
	//regiion (startX,startY) - (startX + step * colum, startY + step * row)
	//
	bool SetRegion(const rect &local_rt, const rect & base_rt, float border_size);
	void InitRegionOverlap(rect& rt);
	inline slice* 	GetSlice(int index) const { return pTable + index;}
	inline slice* 	GetSlice(int x,int z) const { return pTable + x + z * reg_column;}
	inline int 	GetSliceIndex(slice *pPiece) const {return pPiece - pTable;}
	inline void	GetSlicePos(slice *pPiece, int &x,int &z) const 
	{
		x =(int)( (pPiece->slice_range.left - grid_region.left + 0.1f) * inv_step );
		z =(int)( (pPiece->slice_range.top - grid_region.top + 0.1f) * inv_step );
	}
	inline void Index2Pos(int index, int &x,int &z) const { x = index % reg_column; z = index / reg_column;}
	inline bool IsOutsideGrid(float x,float z) const { return grid_region.IsOut(x,z);}
	inline bool IsLocal(float x,float z) const { return local_region.IsIn(x,z);}
public:
	slice * Locate(float x,float z) const;
	slice * Locate(float x,float z,int &rx,int &rz) const;
};

#endif

