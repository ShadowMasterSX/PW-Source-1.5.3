#ifndef __ONLINEGAME_GS_MAPRESMAN_H__
#define __ONLINEGAME_GS_MAPRESMAN_H__

#include <string>

#include <vector.h>
#include <arandomgen.h>
#include <common/types.h>

class CNPCGenMan;
class CTerrain;
namespace NPCMoveMap
{
	class CMap;
}
class trace_manager2;

class world;
class gplayer_imp;

struct map_piece_desp	//地图片的描述信息
{
	int type;
	//...
	int joint_mask;		// 联通性mask  1左2上4右8下
};

enum
{
	MAPRES_TYPE_ORIGIN,
	MAPRES_TYPE_RANDOM,
	MAPRES_TYPE_MAZE,
	MAPRES_TYPE_SEQUENCE,
	MAPRES_TYPE_SOLO_CHALLENGE,

	MAPRES_TYPE_COUNT
};

struct npcgen_data_node_t
{
	CNPCGenMan* npcgen;
	unsigned char blockid;
	A3DVECTOR offset;
	npcgen_data_node_t() : npcgen(NULL),blockid(0),offset(0.f,0.f,0.f) {}
	npcgen_data_node_t(CNPCGenMan* ng,unsigned char bid,const A3DVECTOR& of) : npcgen(ng), blockid(bid), offset(of) {}
};

typedef abase::vector<npcgen_data_node_t > npcgen_data_list;

struct maze_info
{
	int start_idx; 
	int start_dir;
	int end_idx;
	int end_dir;
	int step_min;
	int step_max;
	int branch_min;
	int branch_max;
	int branch_block_min;
	int branch_block_max;
	int ring_min;
	int ring_max;
	int empty_piece;
};

struct map_res
{
	float width;
	float height;
	A3DVECTOR bpos_offset;
	maze_info _maze_info;
	abase::vector<map_piece_desp> _piece_desps;
	abase::vector<A3DVECTOR> _offset_info;
	map_res() : width(0.f),height(0.f),bpos_offset(0,0,0)
	{
		memset(&_maze_info, 0 , sizeof(_maze_info));
	}
};

class MapResManager;
class map_generator
{
protected:
	int _row;						//生成的地图行数		
	int _col;						//生成的地图列数
	float _piece_width;				//小块的长
	float _piece_height;			//小块的宽
	float _map_left;				//左上角x值
	float _map_top;					//左上角z值
	abase::vector<int> _piece_indexes;	//生成的地图每一个小片对应于原始资源的索引
	map_generator():_row(0),_col(0),_piece_width(0.f),_piece_height(0.f),_map_left(0.f),_map_top(0.f) {}
public:
	virtual ~map_generator(){}
	int GetRow() const { return _row; }
	int GetCol() const { return _col; }
	const abase::vector<int> & GetPieceIndexes() const { return _piece_indexes; }
public:
	virtual bool Generate(const rect & region, const map_res& mapres) = 0;
	virtual void SyncPlayerWorldGen(gplayer_imp* pPlayer) const {}
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag) const { return false; }
	virtual bool SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos) const { return false; }	
public:
	bool Init(const rect & region, float piece_width, float piece_height)
	{
		//不支持区域中心不为(0,0)的情况
		float centerx = (region.left + region.right)*0.5f;
		float centerz = (region.top + region.bottom)*0.5f;
		if(centerx < -1e-6 || centerx > 1e-6 || centerz < -1e-6 || centerz > 1e-6) return false;

		_row = (int)(region.Height()/piece_height + 0.9f);
		_col = (int)(region.Width()/piece_width + 0.9f);
		_piece_width = piece_width;
		_piece_height = piece_height;
		_map_left = -_piece_width * _col * 0.5f;
		_map_top = _piece_height * _row * 0.5f;
		ASSERT(_row > 0 && _col > 0);
		return true;
	}
	virtual int GetBlockID(float x, float z, world * pWorld = NULL) const
	{
		int u = int((x - _map_left)/_piece_width);
		int v = int((_map_top - z)/_piece_height);
		return v*_col + u + 1;
	}// block 0 for origin
	virtual int GetRoomIndex(float x, float z) const { return 0; }
public:
	static A3DVECTOR CalcCenterOffset(int col,int row,int maxcol,int maxrow, float piece_width, float piece_height)
	{
		// cur block center (pw*col+ pw*/2, ph*row+ ph*/2)
		// origin center (pw*maxcol/2, ph*maxrow/2)
		return A3DVECTOR(piece_width*0.5f*(2*col+1-maxcol),
				0,	
				-piece_height*0.5f*(2*row+1-maxrow));  // negative Y coordinate
	}
};

class random_map_generator : public map_generator
{
public:
	virtual bool Generate(const rect & region, const map_res& mapres);
};

class sequence_map_generator : public map_generator
{
public:
	virtual bool Generate(const rect & region, const map_res& mapres);
	virtual void SyncPlayerWorldGen(gplayer_imp* pPlayer) const;
};

class solo_challenge_map_generator : public map_generator
{
public:
	virtual int GetBlockID(float x, float z, world * plane) const;
	virtual bool Generate(const rect & region, const map_res& mapres){return true;};
};

/*
 * 初始进入坐标 SetIncomingPlayerPos 由 map_desp.sev 给出
 * 回城坐标 GetTownPosition 1处于起点块返回GetLogoutPos 2处于其他块返回SetIncomingPlayerPos坐标
 */ 
class maze_map_generator : public map_generator
{
public:
	virtual bool Generate(const rect & region, const map_res& mapres);
	virtual void SyncPlayerWorldGen(gplayer_imp* pPlayer) const;
	virtual bool SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos) const;
	virtual bool GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag) const;

	virtual int GetRoomIndex(float x, float z) const	
	{
		int block = GetBlockID(x,z);
		return (block > _col*_row || block < 1) ?  0 : _room_indexes[block-1];
	}
protected:
	bool _GenMaze(const map_res& mapres);
	int _GetAppropriatePiece(const abase::vector<map_piece_desp> & piece_desps,int joint_mask,int type) const;
protected:	
	A3DVECTOR _birth_pos;			// 出生点
	abase::vector<int> _room_indexes; // 每一块对应主线的序号1开始，非主线为0
};

class MapResManager
{
	int _mapres_type;
	map_res _mapres_info;
	
	abase::vector<CTerrain *> _terrain_pieces;
	abase::vector<NPCMoveMap::CMap *> _movemap_pieces;
	abase::vector<trace_manager2 *> _traceman_pieces;
	
	struct
	{
		CNPCGenMan* main_data;  
		abase::vector<CNPCGenMan *> spawn_pieces;
	}_npcgen_info;

public:
	MapResManager():_mapres_type(-1)
	{
		_npcgen_info.main_data = NULL;
	}
	~MapResManager();
	/*
	 *	@param servername e.g. "is05"
	 *	@param base_path e.g. "/home/game/game/config/a05/"
	 *	@param region, local region of the grid
	 */
	int Init(std::string servername, std::string base_path, const rect & region, world * plane);

	int GetType() const { return _mapres_type; }
	void SetType(int t);
	const map_res& GetMapResInfo() const { return _mapres_info; } 
	CTerrain * GetUniqueTerrain(){ ASSERT(_mapres_type == MAPRES_TYPE_ORIGIN || _mapres_type == MAPRES_TYPE_SOLO_CHALLENGE); return _terrain_pieces[0]; }
	NPCMoveMap::CMap * GetUniqueMoveMap(){ ASSERT(_mapres_type == MAPRES_TYPE_ORIGIN || _mapres_type == MAPRES_TYPE_SOLO_CHALLENGE); return _movemap_pieces[0]; }
	trace_manager2 * GetUniqueTraceMan(){ ASSERT(_mapres_type == MAPRES_TYPE_ORIGIN || _mapres_type == MAPRES_TYPE_SOLO_CHALLENGE); return _traceman_pieces[0]; }

	CTerrain * CreateTerrain(map_generator * pGenerator);
	NPCMoveMap::CMap * CreateMoveMap(map_generator * pGenerator, world * plane);
	trace_manager2 * CreateTraceMan(map_generator * pGenerator);
	bool BuildNpcGenerator(world* pWorld);	

};


#endif
