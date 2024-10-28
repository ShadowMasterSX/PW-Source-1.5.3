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

struct map_piece_desp	//��ͼƬ��������Ϣ
{
	int type;
	//...
	int joint_mask;		// ��ͨ��mask  1��2��4��8��
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
	int _row;						//���ɵĵ�ͼ����		
	int _col;						//���ɵĵ�ͼ����
	float _piece_width;				//С��ĳ�
	float _piece_height;			//С��Ŀ�
	float _map_left;				//���Ͻ�xֵ
	float _map_top;					//���Ͻ�zֵ
	abase::vector<int> _piece_indexes;	//���ɵĵ�ͼÿһ��СƬ��Ӧ��ԭʼ��Դ������
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
		//��֧���������Ĳ�Ϊ(0,0)�����
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
 * ��ʼ�������� SetIncomingPlayerPos �� map_desp.sev ����
 * �س����� GetTownPosition 1�������鷵��GetLogoutPos 2���������鷵��SetIncomingPlayerPos����
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
	A3DVECTOR _birth_pos;			// ������
	abase::vector<int> _room_indexes; // ÿһ���Ӧ���ߵ����1��ʼ��������Ϊ0
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
