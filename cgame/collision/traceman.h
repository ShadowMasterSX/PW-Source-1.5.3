#ifndef __ONLINE_GAME_GS_COLLISION_MAN_H__
#define __ONLINE_GAME_GS_COLLISION_MAN_H__

#include <common/types.h>
#include <vector.h>

namespace SvrCD {
	class CBrushMan;
	class CNmdTree;
	class CSMTree;
	class CNmdChd;
}
class trace_manager
{
//	SvrCD::CBrushMan * _brush_man;
//	SvrCD::CBrushMan * _brush_man2;
//	CNmdTree * _nmd_tree;
	SvrCD::CBrushMan * _brush_man;
	SvrCD::CSMTree * _sm_tree;
	int _element_count;
	int _sm_ref;

	static SvrCD::CNmdChd *_nmd_element;

public:
	trace_manager();
	~trace_manager();

	void Attach(const trace_manager * rhs);

	bool Load(const char * filename);
	bool Valid() const { return _brush_man;}
	bool AABBTrace(const A3DVECTOR & start, const A3DVECTOR & offset, const A3DVECTOR & ext, bool & in_solid, float &ratio, const abase::vector<char>* element_flags=NULL);

	static bool LoadElement(const char * filename);	//装载单独文件
	static void ReleaseElement();
	int RegisterElement(int tid, int mid, const A3DVECTOR & pos, float dir0, float dir, float up);
	void EnableElement(int cid, bool active, abase::vector<char>* element_flags=NULL);
	void Compare();
	int GetElementCount(){ return _element_count; }

};

////////////////////////////////////////////////////////////////////

//分片版本
class trace_manager2
{
	struct brush_data
	{
		SvrCD::CBrushMan * brush_man;
		SvrCD::CSMTree * sm_tree;
		int sm_ref;
		brush_data():brush_man(NULL), sm_tree(NULL), sm_ref(0){}
	};
	
	bool _valid;
	float _submap_width;
	float _submap_height;
	float _submap_width_inv;
	float _submap_height_inv;
	int	_row;
	int _col;
	float _map_left;
	float _map_top;
	int _element_count;

	abase::vector<brush_data> _submap_brushes;

	static SvrCD::CNmdChd *_nmd_element;
	
	inline int GetBlockIndex(const A3DVECTOR & pos)
	{
		if(_submap_brushes.size() == 1) return 0;

		int u = (int)((pos.x - _map_left) * _submap_width_inv);
		int v = (int)((_map_top - pos.z) * _submap_height_inv);
		if(u < 0) u = 0;
		if(u > _col - 1) u = _col - 1;
		if(v < 0) v = 0;
		if(v > _row - 1) v = _row - 1;
		return v * _col + u;
	}
public:
	trace_manager2();
	~trace_manager2();

	bool Load(float width, float height, const char * filename);
	bool LoadPiece(float width, float height, const char * filename);
	bool Load(int row, int col, const int * piece_indexes, trace_manager2 ** trace_pieces, bool enable_nmdtree = false);
	bool Valid() const { return _valid;}
	bool AABBTrace(const A3DVECTOR & start, const A3DVECTOR & offset, const A3DVECTOR & ext, bool & in_solid, float &ratio, const abase::vector<char>* element_flags=NULL);

	static bool LoadElement(const char * filename);	//装载单独文件
	static void ReleaseElement();
	int RegisterElement(int tid, int mid, const A3DVECTOR & pos, float dir0, float dir, float up);
	void EnableElement(int cid, bool active, abase::vector<char>* element_flags=NULL);
	int GetElementCount(){ return _element_count; }
};


#endif

