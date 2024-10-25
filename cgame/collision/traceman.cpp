#include "traceman.h"
#include "BrushMan.h"
#include "CDBrush.h"
#include "SMTree.h"
#include "NmdChd.h"
#include "A3DQuaternion.h"
#include <ASSERT.h>
#include <map>
#include <vector>
#include <complex>
#include <algorithm>

SvrCD::CNmdChd *trace_manager::_nmd_element = NULL;

trace_manager::trace_manager()
{
	_brush_man = NULL;
	_sm_tree = NULL;
	_sm_ref = 0;
	
	_element_count = 0;
}

trace_manager::~trace_manager()
{
	if(_brush_man) 
	{
		_brush_man->Release();
		delete _brush_man;
	}
	if(!_sm_ref && _sm_tree)
	{
		_sm_tree->Release();
		delete _sm_tree;
	}
}
void trace_manager::Attach(const trace_manager * rhs)
{
	ASSERT(_sm_tree == NULL);
	ASSERT(_brush_man == NULL);
	_sm_tree = rhs->_sm_tree;
	_sm_ref = 1;

	_brush_man = new SvrCD::CBrushMan();
	_brush_man->SetSmTree(_sm_tree);
}

bool 
trace_manager::Load(const char * filename)
{
	ASSERT(_sm_tree == NULL);
	ASSERT(_brush_man == NULL);
	_sm_tree = new SvrCD::CSMTree();
	if(!_sm_tree->Load(filename))
	{
		delete _sm_tree;
		return false;
	}

	_brush_man = new SvrCD::CBrushMan();
	_brush_man->SetSmTree(_sm_tree);

	return true;
}

bool 
trace_manager::LoadElement(const char * filename)
{
	if(_nmd_element) return false;
	_nmd_element = new SvrCD::CNmdChd();
	if(!_nmd_element->Load(filename))
	{
		delete _nmd_element;
		_nmd_element = NULL;
		return false;
	}
	return true;
}

void 
trace_manager::ReleaseElement()
{
	if(!_nmd_element) return;
	_nmd_element->Release();
	delete _nmd_element;
	_nmd_element = NULL;
}

bool 
trace_manager::AABBTrace(const A3DVECTOR & start, const A3DVECTOR & offset, const A3DVECTOR & ext, bool & in_solid, float & ratio, const abase::vector<char>* element_flags)
{
	if(!_brush_man)  return false;
	
	SvrCD::BrushTraceInfo info;
	info.Init( A3DVECTOR3(start.x,start.y + ext.y ,start.z), A3DVECTOR3(offset.x,offset.y,offset.z), A3DVECTOR3(ext.x,ext.y,ext.z));
	if(!_brush_man->Trace(&info,element_flags)) return false;
	in_solid = info.bStartSolid;
	ratio = info.fFraction;
	return true;
}


int 
trace_manager::RegisterElement(int tid, int mid, const A3DVECTOR & pos, float dir0, float dir1, float rad)
{
	if(!_brush_man) return 0;
	A3DVECTOR3 target(pos.x, pos.y, pos.z);

	A3DVECTOR3 dir,up;
	if(dir1 <1e-3 && rad <1e-3)
	{
		dir.x = (float)cos(dir0);
		dir.y = 0;
		dir.z = (float)sin(dir0);

		up.x = 0;
		up.y = 1;
		up.z = 0;
	}
	else
	{
		//计算Dir和UP
		A3DVECTOR3 vAxis;
		float p = (float)sin(dir1);
		vAxis.x = p * (float)cos(dir0);
		vAxis.z = p * (float)sin(dir0);
		vAxis.y = (float)cos(dir1);

		A3DQUATERNION q(vAxis, rad *256.0f/255.0f);
		A3DMATRIX4 matTran;
		q.ConvertToMatrix(matTran);

		dir = matTran.GetRow(2);
		up  = matTran.GetRow(1);
	}

	if(tid)
	{
		_brush_man->AddNpcMine(_nmd_element,tid, _element_count + 1, target, up, dir);
//		_brush_man2->AddNpcMine(tid, _element_count + 1, target, up, dir);
		_element_count ++;
	}
	else if(mid)
	{
		_brush_man->AddDynObj(_nmd_element,mid, _element_count + 1, target,  up, dir);
//		_brush_man2->AddDynObj(mid, _element_count + 1, target,  up, dir);
		_element_count ++;
	}
	else
	{
		return 0;
	}
	return _element_count;
}

void trace_manager::EnableElement(int cid, bool active, abase::vector<char>* element_flags)
{
	if(!_brush_man) return ;
	if(element_flags)
		(*element_flags)[cid] = active?1:0;
	else
		_brush_man->EnableNmd(cid,active);
}

void trace_manager::Compare()
{
	if(!_brush_man) return;
//	SvrCD::CompareBMan(_brush_man, _brush_man2);
}


////////////////////////////////////////////////////////////////////

SvrCD::CNmdChd *trace_manager2::_nmd_element = NULL;

trace_manager2::trace_manager2():_valid(false),_submap_width(0.f),_submap_height(0.f),_submap_width_inv(0.f),_submap_height_inv(0.f),_row(0),_col(0),_map_left(0.f),_map_top(0.f),_element_count(0)
{

}

trace_manager2::~trace_manager2()
{
	for(size_t i=0; i<_submap_brushes.size(); i++)
	{
		brush_data & brush = _submap_brushes[i];
		if(brush.brush_man)
		{
			brush.brush_man->Release();
			delete brush.brush_man;
		}
		if(!brush.sm_ref && brush.sm_tree)
		{
			brush.sm_tree->Release();
			delete brush.sm_tree;
		}
	}
	_submap_brushes.clear();

}

bool trace_manager2::Load(float width, float height, const char * filename)
{
	SvrCD::CSMTree * sm_tree = new SvrCD::CSMTree();
	if(!sm_tree->Load(filename))
	{
		delete sm_tree;
		return false;
	}

	_submap_width = width;
	_submap_height = height;
	_submap_width_inv = 1 / width;
	_submap_height_inv = 1 / height;
	_row = 1;
	_col = 1;
	_map_left = -0.5f * width;
	_map_top = 0.5f * height;

	brush_data brush;
	brush.sm_tree = sm_tree;
	brush.sm_ref = 0;
	brush.brush_man = new SvrCD::CBrushMan();
	brush.brush_man->SetSmTree(brush.sm_tree, true);
	_submap_brushes.push_back(brush);
	
	_valid = true;
	return true;
}

bool trace_manager2::LoadPiece(float width, float height, const char * filename)
{
	SvrCD::CSMTree * sm_tree = new SvrCD::CSMTree();
	if(!sm_tree->Load(filename))
	{
		delete sm_tree;
		return false;
	}

	_submap_width = width;
	_submap_height = height;
	_submap_width_inv = 1 / width;
	_submap_height_inv = 1 / height;
	_row = 1;
	_col = 1;
	_map_left = -0.5f * width;
	_map_top = 0.5f * height;

	brush_data brush;
	brush.sm_tree = sm_tree;
	brush.sm_ref = 0;
	brush.brush_man = new SvrCD::CBrushMan();
	brush.brush_man->SetSmTree(brush.sm_tree, false);
	_submap_brushes.push_back(brush);
	
	_valid = true;
	return true;
}

bool trace_manager2::Load(int row, int col, const int * piece_indexes, trace_manager2 ** trace_pieces, bool enable_nmdtree)
{
	_submap_width = trace_pieces[0]->_submap_width;
	_submap_height = trace_pieces[0]->_submap_height;
	_submap_width_inv = trace_pieces[0]->_submap_width_inv;
	_submap_height_inv = trace_pieces[0]->_submap_height_inv;
	_row = row;
	_col = col;
	_map_left = -0.5f * col * _submap_width;
	_map_top = 0.5f * row * _submap_height;

	for(int i = 0; i < row; i++)
	{
		for(int j = 0; j < col; j++)
		{
			brush_data brush;
			brush.sm_tree = trace_pieces[piece_indexes[i * col + j]]->_submap_brushes[0].sm_tree;
			brush.sm_ref = 1;
			brush.brush_man = new SvrCD::CBrushMan();
			brush.brush_man->SetSmTree(brush.sm_tree, enable_nmdtree);
			_submap_brushes.push_back(brush);
		}
	}
	
	_valid = true;
	return true;	
}

struct InterSection
{

	struct Pos2D{
		float u;
		float v;
		bool uaxis; // 相交点是否处于u轴上 ，否则则为v轴上
		bool uvc;	// 是否处在uv轴心上, 0.001f范围内
	};
	A3DVECTOR start; // 以左上为0,0点的坐标系中的起始点
	A3DVECTOR offset;
	float width;
	float height;
	int max_index;
	int max_col;
	int max_row;
	std::map<float,Pos2D> cross_point_set;

	enum EIS_DIR{
		ISD_UP,
		ISD_UP_RIGHT,
		ISD_RIGHT,
		ISD_DOWN_RIGHT,
		ISD_DOWN,
		ISD_DOWN_LEFT,
		ISD_LEFT,
		ISD_UP_LEFT
	};

	int GetNearIndex(int col,int row,EIS_DIR dir)
	{
		if(col > max_col || row > max_row || col < 0 ||	row < 0) return -1;

		switch(dir)
		{
			case ISD_UP:
			case ISD_UP_RIGHT:
				return row && col < max_col ? (row -1) * max_col + col : -1; 
			case ISD_RIGHT:
			case ISD_DOWN_RIGHT:	
			case ISD_DOWN:
				return col < max_col && row < max_row ? row * max_col + col : -1;
			case ISD_DOWN_LEFT:	 
			case ISD_LEFT:
				return row < max_row && col ? row * max_col + col - 1 : -1;
			case ISD_UP_LEFT:
				return row && col ? (row -1) * max_col + col -1 : -1;
			default:
				return -1;
		}
	}

	InterSection(const A3DVECTOR & sta, const A3DVECTOR & off,float wdt, float hgt)
		: start(sta), offset(off), width(wdt), height(hgt)
	{
		int start_u = (int) (start.x / width);
		int start_v = (int) (start.z / height);
		int end_u = (int) ((start.x + offset.x) / width);
		int end_v = (int) ((start.z + offset.z) / height);
		float k = offset.z/offset.x;
		float b = start.z - k*start.x;
		
		bool negative_u = offset.x < 0;
		bool negative_v = offset.z < 0;

		int step_u = negative_u ? -1 : 1;
		int step_v = negative_v ? -1 : 1;

		Pos2D cross_point;
		Pos2D cross_point_modify;
		cross_point.v = negative_v ? (start_v - 1)*height : start_v * height;
		cross_point.uaxis = true;

		for(int n = start_v; n != end_v; n+=step_v,	cross_point.v += (step_v * height ))
		{
			cross_point.u = (cross_point.v - b)/k;
			cross_point.uvc = fmod(cross_point.u, width) < 0.001f;
			cross_point_modify = cross_point;
			cross_point_modify.v -= 0.001f; // 0.001f防止float->int时候的误差
			if(cross_point.uvc) cross_point_modify.u += 0.001f;
			cross_point_set[start.horizontal_distance(A3DVECTOR(cross_point.u,0,cross_point.v))] = cross_point_modify; 		
		}
		
		cross_point.u = negative_u ? start_u * width : (start_u + 1) * width;
		cross_point.uaxis = false;
			 
		for(int n = start_u; n != end_u; n+=step_u,	cross_point.u += (step_u * width))
		{
			cross_point.v = cross_point.u *k + b;
			cross_point.uvc = -fmod(cross_point.v, height) < 0.001f;
			cross_point_modify = cross_point;
			cross_point_modify.u += 0.001f; // 0.001f防止float->int时候的误差
			if(cross_point.uvc) cross_point_modify.v -= 0.001f;
			cross_point_set[start.horizontal_distance(A3DVECTOR(cross_point.u,0,cross_point.v))] = cross_point_modify;
		}

	}

	void PushUnique(std::vector<int>& sids,int index)
	{
		if(index < 0 || index >= max_index) return;

		if(sids.end() == std::find(sids.begin(),sids.end(),index))
			sids.push_back(index);
	}	

	void GetInterSectionId(std::vector<int>& sids,int mxc,int mxr)
	{
		max_col = mxc;
		max_row = mxr;
		max_index = max_col * max_row;
		std::map<float,Pos2D>::iterator iter = cross_point_set.begin();
		std::map<float,Pos2D>::iterator iend = cross_point_set.end();		

#define INSERT_SECTION_ID(dir1,dir2) { PushUnique(sids,GetNearIndex(col,row,dir1)); PushUnique(sids,GetNearIndex(col,row,dir2)); }

		for(; iter != iend; ++iter)
		{
			Pos2D& pos = iter->second;
			int col = (int)(pos.u/width);
			int row = (int)(pos.v/-height);

			if(pos.uvc) // 交点过uv轴心
			{
				if(offset.x > 0 && offset.z > 0) // (0,90)
					INSERT_SECTION_ID(ISD_DOWN_LEFT,ISD_UP_RIGHT)
				else if(offset.x < 0 && offset.z > 0)// ( 90, 180)
					INSERT_SECTION_ID(ISD_DOWN_RIGHT,ISD_UP_LEFT)
				else if(offset.x < 0 && offset.z < 0) // (180,270)
					INSERT_SECTION_ID(ISD_UP_RIGHT,ISD_DOWN_LEFT)
				else if(offset.x > 0 && offset.z < 0) // (270,360)
					INSERT_SECTION_ID(ISD_UP_LEFT,ISD_DOWN_RIGHT)
				else if(offset.x == 0 && offset.z > 0) // 90
					INSERT_SECTION_ID(ISD_DOWN,ISD_UP)
				else if(offset.x == 0 && offset.z < 0) // 270
					INSERT_SECTION_ID(ISD_UP,ISD_DOWN)
				else if(offset.z == 0 && offset.x > 0) // 0
					INSERT_SECTION_ID(ISD_LEFT,ISD_RIGHT)
				else if(offset.z == 0 && offset.x < 0)  // 180
					INSERT_SECTION_ID(ISD_RIGHT,ISD_LEFT)
			//	else
					//ASSERT(false);
			}
			else if(pos.uaxis)
			{
				if(offset.z > 0)
					INSERT_SECTION_ID(ISD_DOWN,ISD_UP)
				else
					INSERT_SECTION_ID(ISD_UP,ISD_DOWN)
			}
			else
			{
				if(offset.x > 0)
					INSERT_SECTION_ID(ISD_LEFT,ISD_RIGHT)
				else
					INSERT_SECTION_ID(ISD_RIGHT,ISD_LEFT)
			}
		}
 
#undef INSERT_SECTION_ID 

	}
};

A3DVECTOR GetBlockCenter(int col,int row,int maxcol,int maxrow, float piece_width, float piece_height)
{
	// cur block center (pw*col+ pw*/2, ph*row+ ph*/2)
	// origin center (pw*maxcol/2, ph*maxrow/2)
	return A3DVECTOR(piece_width*0.5f*(2*col+1-maxcol),
			0,	
			-piece_height*0.5f*(2*row+1-maxrow));  // negative Y coordinate
}

bool trace_manager2::AABBTrace(const A3DVECTOR & start, const A3DVECTOR & offset, const A3DVECTOR & ext, bool & in_solid, float &ratio, const abase::vector<char>* element_flags)
{
	if(!_valid) return false;

	A3DVECTOR end = start;
	end += offset;
	
	int start_index = GetBlockIndex(start);
	int end_index = GetBlockIndex(end);

	if(start_index == end_index)
	{
		A3DVECTOR bc = GetBlockCenter(start_index%_col,start_index/_col,_col,_row, _submap_width, _submap_height); // 中心点相对偏移
		SvrCD::BrushTraceInfo info;
		info.Init( A3DVECTOR3(start.x - bc.x,start.y + ext.y ,start.z - bc.z), A3DVECTOR3(offset.x,offset.y,offset.z), A3DVECTOR3(ext.x,ext.y,ext.z));
		if(!_submap_brushes[start_index].brush_man->Trace(&info,element_flags)) return false;

		in_solid = info.bStartSolid;
		ratio = info.fFraction;
		return true;
	}
	else
	{
		InterSection is(A3DVECTOR(start.x - _map_left,0,start.z - _map_top),A3DVECTOR(offset.x,0,offset.z),_submap_width, _submap_height);
		std::vector<int> vt;  // 按方向排序后的经过序号集合
		is.GetInterSectionId(vt,_col,_row);
		
		for(size_t n = 0; n < vt.size(); ++n)
		{
			int index = vt[n];
			ASSERT(index < _col*_row);
		
			A3DVECTOR bc = GetBlockCenter(index%_col,index/_col,_col,_row, _submap_width, _submap_height); // 中心点相对偏移
			SvrCD::BrushTraceInfo info;
			info.Init( A3DVECTOR3(start.x - bc.x,start.y + ext.y ,start.z - bc.z), A3DVECTOR3(offset.x,offset.y,offset.z), A3DVECTOR3(ext.x,ext.y,ext.z));
			if(!_submap_brushes[index].brush_man->Trace(&info,element_flags)) continue;

			in_solid = info.bStartSolid;
			ratio = info.fFraction;
			return true;
		}
		return false;
	}
}

bool trace_manager2::LoadElement(const char * filename)
{
	if(_nmd_element) return false;
	_nmd_element = new SvrCD::CNmdChd();
	if(!_nmd_element->Load(filename))
	{
		delete _nmd_element;
		_nmd_element = NULL;
		return false;
	}
	return true;
}

void trace_manager2::ReleaseElement()
{
	if(!_nmd_element) return;
	_nmd_element->Release();
	delete _nmd_element;
	_nmd_element = NULL;
}

int trace_manager2::RegisterElement(int tid, int mid, const A3DVECTOR & pos, float dir0, float dir1, float rad)
{
	if(!_valid) return 0;
	int index = GetBlockIndex(pos);	
	//判断是否支持动态碰撞
	if(_submap_brushes[index].brush_man->GetNmdTree() == NULL) return 0;
	
	A3DVECTOR bc = GetBlockCenter(index%_col, index/_col, _col, _row, _submap_width, _submap_height);
	A3DVECTOR3 target(pos.x - bc.x, pos.y, pos.z - bc.z);

	A3DVECTOR3 dir,up;
	if(dir1 <1e-3 && rad <1e-3)
	{
		dir.x = (float)cos(dir0);
		dir.y = 0;
		dir.z = (float)sin(dir0);

		up.x = 0;
		up.y = 1;
		up.z = 0;
	}
	else
	{
		//计算Dir和UP
		A3DVECTOR3 vAxis;
		float p = (float)sin(dir1);
		vAxis.x = p * (float)cos(dir0);
		vAxis.z = p * (float)sin(dir0);
		vAxis.y = (float)cos(dir1);

		A3DQUATERNION q(vAxis, rad *256.0f/255.0f);
		A3DMATRIX4 matTran;
		q.ConvertToMatrix(matTran);

		dir = matTran.GetRow(2);
		up  = matTran.GetRow(1);
	}

	if(tid)
	{
		_submap_brushes[index].brush_man->AddNpcMine(_nmd_element,tid, _element_count + 1, target, up, dir);
		_element_count ++;
	}
	else if(mid)
	{
		_submap_brushes[index].brush_man->AddDynObj(_nmd_element,mid, _element_count + 1, target,  up, dir);
		_element_count ++;
	}
	else
	{
		return 0;
	}
	return _element_count;
}

void trace_manager2::EnableElement(int cid, bool active, abase::vector<char>* element_flags)
{
	if(!_valid) return;
	if(element_flags)
		(*element_flags)[cid] = active?1:0;
	else
	{
		//效率有点低，可以考虑建立cid->brush idx查询表
		for(size_t i=0; i<_submap_brushes.size(); i++)
		{
			_submap_brushes[i].brush_man->EnableNmd(cid,active);
		}
	}
}

