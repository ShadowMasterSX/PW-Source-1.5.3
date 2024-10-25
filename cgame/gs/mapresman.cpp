#include <strings.h>
#include <ASSERT.h>
#include <conf.h>
#include "dbgprt.h"

#include "terrain.h"
#include "pathfinding/pathfinding.h"
#include "template/npcgendata.h"
#include "template/el_randommapinfo.h"
#include "../collision/traceman.h"
#include "world.h"
#include "mapresman.h"
#include "random_maze.h"
#include "player_imp.h"
MapResManager::~MapResManager()
{
	abase::clear_ptr_vector(_terrain_pieces);
	abase::clear_ptr_vector(_movemap_pieces);
	abase::clear_ptr_vector(_traceman_pieces);
	abase::clear_ptr_vector(_npcgen_info.spawn_pieces);
	if(_npcgen_info.main_data) delete _npcgen_info.main_data;
}

void MapResManager::SetType(int t)
{
	if(!player_template::GetDebugMode() || !t || t >= MAPRES_TYPE_COUNT ) return;
	if(_mapres_type != MAPRES_TYPE_ORIGIN) _mapres_type = t; // 禁止老版地图重组
}

int MapResManager::Init(std::string servername, std::string base_path, const rect & region, world * plane)
{
	ONET::Conf *conf = ONET::Conf::GetInstance();
	std::string section = "Terrain_";
	section += servername;

	std::string str = conf->find(section, "mapres_type"); 
	if(str == "random")
		_mapres_type = MAPRES_TYPE_RANDOM;
	else if(str == "maze")
		_mapres_type = MAPRES_TYPE_MAZE;
	else
		_mapres_type = MAPRES_TYPE_ORIGIN;

	__PRINTINFO("加载地图资源, servername:%s base_path:%s mapres_type:%d\n", servername.c_str(), base_path.c_str(), _mapres_type);

	if(_mapres_type == MAPRES_TYPE_ORIGIN)
	{
		//加载地形
		TERRAINCONFIG config;
		config.nAreaWidth = atoi(conf->find(section,"nAreaWidth").c_str());
		config.nAreaHeight = atoi(conf->find(section,"nAreaHeight").c_str());
		config.nNumAreas = atoi(conf->find(section,"nNumAreas").c_str());
		config.nNumCols = atoi(conf->find(section,"nNumCols").c_str());
		config.nNumRows =atoi(conf->find(section,"nNumRows").c_str());
		config.vGridSize = atof(conf->find(section,"vGridSize").c_str());
		config.vHeightMin = atof(conf->find(section,"vHeightMin").c_str());
		config.vHeightMax = atof(conf->find(section,"vHeightMax").c_str());
		memset(config.szMapPath,0,sizeof(config.szMapPath));
		std::string path = base_path + conf->find(section,"szMapPath");
		strncpy(config.szMapPath, path.c_str(),sizeof(config.szMapPath) - 1);

		CTerrain * pTerrain = new CTerrain();
		ASSERT(region.left <= region.right && region.top <= region.bottom);
		if(!pTerrain->Init(config,region.left,region.top,region.right,region.bottom))
		{
			__PRINTINFO("无法初始化地形数据\n");
			return -101;
		}
		_terrain_pieces.push_back(pTerrain);
		//加载寻路
		std::string str1 = base_path + conf->find("MoveMap","Path");
		std::string str2 = base_path + conf->find("MoveMap","WaterPath");
		std::string str3 = base_path + conf->find("MoveMap","AirPath");

		NPCMoveMap::CMap * movemap = path_finding::InitMoveMap(str1.c_str(),str2.c_str(),str3.c_str(),plane);
		if(!movemap)
		{
			__PRINTF("无法读入NPC通路图或者无法读入水域图㘎n");
			return -102;
		}
		_movemap_pieces.push_back(movemap);

		//加载碰撞
		std::string trace_path = base_path + conf->find("Template","CollisionFile");
		float map_width = config.nAreaWidth * config.vGridSize * config.nNumCols;
		float map_height = config.nAreaHeight * config.vGridSize * config.nNumRows;
		trace_manager2 * pTraceMan = new trace_manager2();
		if(!pTraceMan->Load(map_width, map_height, trace_path.c_str()))
		{
			printf("加载凸包数据'%s'失败\n",trace_path.c_str());
		}
		else
		{
			printf("加载'%s'完成\n",trace_path.c_str());
		}
		_traceman_pieces.push_back(pTraceMan);
		//加载NPCGen
		std::string  npcgenfile = base_path + conf->find("Template","NPCGenFile");
		_npcgen_info.main_data = new CNPCGenMan();
		
		if(!_npcgen_info.main_data->Load(npcgenfile.c_str()))
		{
			__PRINTINFO("无法打开 主npc分布文件\n");
			return -103;
		}
		__PRINTINFO("主地图一共有%d个怪物区域\n", _npcgen_info.main_data->GetGenAreaNum());
	}
	else
	{
		int piece_num = atoi(conf->find(section,"nPiece").c_str());	
		TERRAINCONFIG config;
		config.nAreaWidth = atoi(conf->find(section,"nAreaWidth").c_str());
		config.nAreaHeight = atoi(conf->find(section,"nAreaHeight").c_str());
		config.nNumAreas = 1;
		config.nNumCols = 1;
		config.nNumRows = 1;
		config.vGridSize = atof(conf->find(section,"vGridSize").c_str());
		config.vHeightMin = atof(conf->find(section,"vHeightMin").c_str());
		config.vHeightMax = atof(conf->find(section,"vHeightMax").c_str());
		memset(config.szMapPath,0,sizeof(config.szMapPath));
		std::string path = base_path + conf->find(section,"szMapPath");
		strncpy(config.szMapPath, path.c_str(),sizeof(config.szMapPath) - 1);

		//加载地图片描述信息
		_mapres_info.width = config.nAreaWidth * config.vGridSize;
		_mapres_info.height = config.nAreaHeight * config.vGridSize;

		//加载随机生成条件
		CRandMapProp randmap_prop;
		std::string desp_file = base_path + conf->find("Template","MapDespFile");
		randmap_prop.LoadSev(desp_file.c_str());
		ASSERT(piece_num==randmap_prop.GetGridCount()&&"gs.conf和map_desp_sev描述地图大小要一致"); 

		const CRandMapProp::FILEHEADER& mz_info = randmap_prop.GetHeader();
		_mapres_info.bpos_offset = A3DVECTOR(mz_info.fPosX,mz_info.fPosY,mz_info.fPosZ);
		_mapres_info._maze_info.start_idx = -1;
		_mapres_info._maze_info.start_dir = -1;	
		_mapres_info._maze_info.end_idx = -1;	
		_mapres_info._maze_info.end_dir = -1;	
		_mapres_info._maze_info.step_min = mz_info.mainLineMinLen;
		_mapres_info._maze_info.step_max = mz_info.mainLineMaxLen;
		_mapres_info._maze_info.branch_min = mz_info.branchLineNumMin;
		_mapres_info._maze_info.branch_max = mz_info.branchLineNumMax;
		_mapres_info._maze_info.branch_block_min = mz_info.branchLineMinLen;
		_mapres_info._maze_info.branch_block_max = mz_info.branchLineMaxLen;
		_mapres_info._maze_info.empty_piece = piece_num - 1; // 默认规则最后一个房间为空

		for(int i=0; i<piece_num; i++)
		{
			map_piece_desp desp;

			//load file ...
			CRandMapProp::MAP_INFO info;
			randmap_prop.GetGirdProp(i,info);	
			desp.type = info.type;
			desp.joint_mask = info.connection;
			_mapres_info._piece_desps.push_back(desp);
		}
		//加载地图片地形	
		for(int i=0; i<piece_num; i++)
		{
			CTerrain * pTerrain = new CTerrain();
			if(!pTerrain->InitPiece(config,i))
			{
				__PRINTINFO("无法初始化地形数据\n");
				return -201;
			}
			_terrain_pieces.push_back(pTerrain);
		}
		//加载地图片寻路
		std::string str1 = base_path + conf->find("MoveMap","Path");
		std::string str2 = base_path + conf->find("MoveMap","WaterPath");
		std::string str3 = base_path + conf->find("MoveMap","AirPath");

		for(int i=0; i<piece_num; i++)
		{
			NPCMoveMap::CMap * movemap = path_finding::InitMoveMapPiece(str1.c_str(),str2.c_str(),str3.c_str(),plane,i);
			if(!movemap)
			{
				__PRINTF("无法读入NPC通路图或者无法读入水域图or天空图\n");
				return -202;
			}
			_movemap_pieces.push_back(movemap);
		}
		//加载地图片碰撞
		for(int i=0; i<piece_num; i++)
		{
			char trace_path[256];
			sprintf(trace_path, "%smapbht/%d.bht", base_path.c_str(), i+1);
			float piece_width = config.nAreaWidth * config.vGridSize;
			float piece_height = config.nAreaHeight * config.vGridSize;
			trace_manager2 * pTraceMan = new trace_manager2();
			if(!pTraceMan->LoadPiece(piece_width, piece_height, trace_path))
			{
				printf("加载凸包数据'%s'失败\n",trace_path);
			}
			else
			{
				printf("加载'%s'完成\n",trace_path);
			}
			_traceman_pieces.push_back(pTraceMan);
		}
		//加载NPCGen
		std::string  npcgenfile = base_path + conf->find("Template","NPCGenFile");
		_npcgen_info.main_data = new CNPCGenMan();
		
		if(!_npcgen_info.main_data->Load(npcgenfile.c_str()))
		{
			__PRINTINFO("无法打开 主npc分布文件\n");
			return -203;
		}
		__PRINTINFO("主地图一共有%d个怪物区域\n", _npcgen_info.main_data->GetGenAreaNum());
			
		for(int i=0; i<piece_num; i++)
		{
			char szSpawnFile[256];
			sprintf(szSpawnFile, "%snpcgen/npcgen_%d.data",base_path.c_str(),i+1);
			CNPCGenMan* spawndata = new CNPCGenMan();
			if(!spawndata->Load(szSpawnFile))
			{
				__PRINTINFO("无法打开 分支地图%dnpc分布文件\n",i);
				return -203;
			}
			__PRINTINFO("分支地图%d一共有%d个怪物区域\n", i,spawndata->GetGenAreaNum());
			_npcgen_info.spawn_pieces.push_back(spawndata);
		}
	}
	
	return 0;
}

CTerrain * MapResManager::CreateTerrain(map_generator * pGenerator)
{
	CTerrain * pTerrain = new CTerrain();
	if(!pTerrain->Init(pGenerator->GetRow(), pGenerator->GetCol(), pGenerator->GetPieceIndexes().begin(), _terrain_pieces.begin()))
	{
		delete pTerrain;
		return NULL;
	}
	return pTerrain;
}

NPCMoveMap::CMap * MapResManager::CreateMoveMap(map_generator * pGenerator, world * plane)
{
	NPCMoveMap::CMap * movemap = path_finding::InitMoveMap(plane, pGenerator->GetRow(), pGenerator->GetCol(), pGenerator->GetPieceIndexes().begin(), _movemap_pieces.size(), _movemap_pieces.begin());
	return movemap;
}

trace_manager2 * MapResManager::CreateTraceMan(map_generator * pGenerator)
{
	trace_manager2 * pTraceMan = new trace_manager2;
	pTraceMan->Load(pGenerator->GetRow(), pGenerator->GetCol(), pGenerator->GetPieceIndexes().begin(), _traceman_pieces.begin(), false);
	return pTraceMan;
}

bool MapResManager::BuildNpcGenerator(world* pWorld)
{
	const map_generator * pGenerator = pWorld->GetMapGen();

	if(_mapres_type == MAPRES_TYPE_ORIGIN)
	{
		return pWorld->InitNPCGenerator(*_npcgen_info.main_data);
	}
	else
	{
		const abase::vector<int>& piece_indexes = pGenerator->GetPieceIndexes();
		npcgen_data_list spawn_list;
		spawn_list.reserve(piece_indexes.size());
		npcgen_data_node_t spawn_data;

		int row = pGenerator->GetRow();
		int col = pGenerator->GetCol();
	
		for(int v = 0; v < row; ++v)
		{
			for(int u = 0; u < col; ++u)
			{
				int idx = v * col + u;
				int destidx = piece_indexes[idx];
				if((size_t)destidx >= _npcgen_info.spawn_pieces.size())
					return false;
				
				spawn_data.npcgen = _npcgen_info.spawn_pieces[destidx];
				spawn_data.blockid = idx + 1;
				spawn_data.offset = map_generator::CalcCenterOffset(u,v,col,row,_mapres_info.width,_mapres_info.height);

				spawn_list.push_back(spawn_data);
			}
		}
	
		return pWorld->InitNPCGenerator(*_npcgen_info.main_data, spawn_list);
	}

	return true;
}
//////////////////////////////////////////////////////////////
bool random_map_generator::Generate(const rect & region, const map_res& mapres) 
{
	if(!Init(region, mapres.width, mapres.height)) return false;
	size_t piece_num = mapres._piece_desps.size();
	ASSERT(piece_num);
	_piece_indexes.insert(_piece_indexes.end(), _col*_row, -1);	
	for(size_t i=0; i<_piece_indexes.size(); i++)
	{
		_piece_indexes[i] = abase::Rand(0, piece_num-1);
	}
	return true;
}
//////////////////////////////////////////////////////////////
bool sequence_map_generator::Generate(const rect & region, const map_res& mapres) 
{
	if(!Init(region, mapres.width, mapres.height)) return false;
	size_t piece_num = mapres._piece_desps.size();
	ASSERT(piece_num == (size_t)(_col*_row) && "地图资源和生成大小不匹配");
	_piece_indexes.insert(_piece_indexes.end(), piece_num, -1);	
	for(size_t i=0; i< piece_num; i++)
	{
		_piece_indexes[i] = i;
	}
	return true;
}

void sequence_map_generator::SyncPlayerWorldGen(gplayer_imp* pPlayer) const
{
	gplayer_dispatcher* runner = (gplayer_dispatcher *)pPlayer->_runner;
	runner->randommap_order_init(_row,_col,_piece_indexes.begin());
}

//////////////////////////////////////////////////////////////
bool maze_map_generator::Generate(const rect & region, const map_res& mapres)
{
	if(!Init(region, mapres.width, mapres.height)) return false;
	if(!_GenMaze(mapres)) return false;
	return true;
}

bool maze_map_generator::_GenMaze(const map_res& mapres)
{
	struct timeval tv_beg,tv_end;
	gettimeofday(&tv_beg,NULL);
	RandomMaze::MazeGen generator;
	const maze_info& minfo = mapres._maze_info;
	int ret = generator.Init(_col,_row - 1, // 最后一行为固定的隐藏房间
	                    minfo.start_idx, (RandomMaze::Direction)minfo.start_dir,
	                    minfo.end_idx, (RandomMaze::Direction)minfo.end_dir,
	                    minfo.step_min, minfo.step_max,
	                    minfo.branch_min, minfo.branch_max,
	                    minfo.branch_block_min, minfo.branch_block_max,
	                    minfo.ring_min, minfo.ring_max);
	if(ret != 0)
	{
		__PRINTINFO("随机迷宫生成参数错误retcode=%d(%s)\n", 
		         ret,RandomMaze::MazeGen::TranslateErrorCode(ret).c_str());
		return false;
	}
	//生成的迷宫不一定严格满足所有指定参数
	ret = generator.Generate(true);
	gettimeofday(&tv_end,NULL);
	int costtime = 1000000*(tv_end.tv_sec - tv_beg.tv_sec) + tv_end.tv_usec - tv_beg.tv_usec;

	if (ret != 0 )
	{
		__PRINTINFO("随机迷宫生成警告retcode=%d(%s)\n", 
				ret,RandomMaze::MazeGen::TranslateErrorCode(ret).c_str());
	}

	const RandomMaze::Maze& maze = generator.GetMaze();
	__PRINTINFO("生成%dx%d 分支数%d 分支房间数%d 环数%d 主路径总步数%d 的随机迷宫耗时%d 微秒\n",
			maze.GetHorizRoomCount(),maze.GetVertRoomCount(),
			maze.GetBranchCount(),maze.GetBranchRoomCount(),
			maze.GetRingCount(),maze.GetStepCount(),costtime);

	_piece_indexes.insert(_piece_indexes.end(), _col*_row, minfo.empty_piece);
	_room_indexes.insert(_room_indexes.end(), _col*_row, 0);
	_birth_pos = A3DVECTOR(0,0,0);

	std::vector<int>::const_iterator citer = generator.GetMainPath().begin(); 
	std::vector<int>::const_iterator ciend = generator.GetMainPath().end(); 

	for(int i = 1; citer != ciend; ++citer, ++i)
	{
		_room_indexes[*citer] = i; 
	}

	int total_room = _col*(_row-1);
	
	for(int index = 0; index < total_room; ++index)
	{
		const RandomMaze::Room& room = maze.GetRoom(index);
		int roommask = room.GetDoorMask();
		
		if(!roommask) continue;
		
		int roomtype = CRandMapProp::GRID_TYPE_NORMAL;

		if(index == maze.GetEntranceRoomNo())
		{
			roomtype = CRandMapProp::GRID_TYPE_START;
			_birth_pos = CalcCenterOffset(index%_col, index/_col, _col, _row, mapres.width, mapres.height);
			_birth_pos += mapres.bpos_offset;
		}
		else if(index == maze.GetExitRoomNo())
			roomtype = CRandMapProp::GRID_TYPE_END;

		_piece_indexes[index] = _GetAppropriatePiece(mapres._piece_desps,roommask,roomtype);

		if(_piece_indexes[index] < 0) 
		{
			GLog::log(GLOG_ERR,"随机地图未找到匹配房间%d 联通条件%d 类型%d",index,roommask,roomtype);
			_piece_indexes[index] = minfo.empty_piece; 
		}
	}

	generator.ShowRoom();

	// 资源固定的房间
	ASSERT(_col*_row > 4 && "隐藏规则房间需要符合个数");
	for(int index = _col*_row  - 1; index > _col*_row - 5; --index)
		_piece_indexes[index] = index; 

	//	输出结果
	for(int v = 0; v < _row; ++v)
	{
		for(int u = 0; u < _col; ++u)
		{
			printf("[%02d]",_piece_indexes[u+v*_col]);
		}
		printf("\n");
	}

	return true;
}

int maze_map_generator::_GetAppropriatePiece(const abase::vector<map_piece_desp> & piece_desps,int joint_mask, int type) const
{
	abase::vector<int> equal_joint_pieces;
	abase::vector<int> close_joint_pieces;

	for(int i = 0; i < (int)piece_desps.size(); ++i)
	{
		if(piece_desps[i].type != type) continue;
		
		int mask = piece_desps[i].joint_mask;
		if( mask == joint_mask)
		{
			equal_joint_pieces.push_back(i);
		}
		else if ( (mask & joint_mask ) == joint_mask )
		{
			close_joint_pieces.push_back(i);
		}
	}

	int iNum;
	if (equal_joint_pieces.size())
	{
		int rand_idx = 0;
		iNum = (int)equal_joint_pieces.size();

		if(iNum > 1)
			rand_idx = abase::Rand(rand_idx,iNum-1);

		return equal_joint_pieces[rand_idx];
	}

	if (close_joint_pieces.size())
	{
		int rand_idx = 0;
		iNum = (int)close_joint_pieces.size();

		if(iNum > 1)
			rand_idx = abase::Rand(rand_idx,iNum-1);

		GLog::log(GLOG_ERR,"警告：随机地图未找到最优联通匹配类型%d 房间%d 条件%d 实际%d",
				type, rand_idx, joint_mask, piece_desps[close_joint_pieces[rand_idx]].joint_mask );

		return close_joint_pieces[rand_idx];
	}

	return -1;
}

void maze_map_generator::SyncPlayerWorldGen(gplayer_imp* pPlayer) const
{
	gplayer_dispatcher* runner = (gplayer_dispatcher *)pPlayer->_runner;
	runner->randommap_order_init(_row,_col,_piece_indexes.begin());
}

bool maze_map_generator::SetIncomingPlayerPos(gplayer * pPlayer, const A3DVECTOR & origin_pos) const
{
	pPlayer->pos = _birth_pos;
	return true;
}

bool maze_map_generator::GetTownPosition(gplayer_imp *pImp, const A3DVECTOR &opos, A3DVECTOR &pos, int & tag) const 
{ 
	if(!pImp->_parent->IsZombie() && GetRoomIndex(opos.x,opos.z) == 1) // 处于起点块 非死亡状态
	{
		world_manager::GetInstance()->GetLogoutPos(pImp,tag,pos);
	}
	else if(GetBlockID(opos.x,opos.z) > 60) // 隐藏房间配置随precinct.sev
	{
		return false;
	}
	else
	{
		pos = _birth_pos;
		tag = world_manager::GetWorldTag();
	}
	return true; 
}

