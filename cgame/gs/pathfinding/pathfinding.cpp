#include "pathfinding.h"
#include "../world.h"
#include "../worldmanager.h"
#include "GlobalWaterAreaMap.h"
#include "GlobalSPMap.h"

namespace path_finding
{
int follow_target::_detail_param[3][4] = {{300,20,50,0},{600,40,90,0},{900,60,120,0}};

NPCMoveMap::CMap* InitMoveMap(const char * move_map_path , const char * water_map_path, const char * air_map, world * plane)
{
	//return g_NPCMoveMap.Load(move_map_path) && WaterAreaMap::LoadData(water_map_path) && CGlobalSPMap::GetInstance()->Load(air_map);
	PathfindingMap* p = new PathfindingMap(plane);
	if(p->Init(move_map_path, water_map_path, air_map))
		return p;
	delete p;
	return NULL;
}

NPCMoveMap::CMap* InitMoveMapPiece(const char * move_map_path , const char * water_map_path, const char * air_map, world * plane, int piece_idx)
{
	PathfindingMap* p = new PathfindingMap(plane);
	if(p->InitPiece(move_map_path, water_map_path, air_map, piece_idx))
		return p;
	delete p;
	return NULL;
}

NPCMoveMap::CMap* InitMoveMap(world * plane, int row, int col, const int * piece_indexes, size_t pieces_count, NPCMoveMap::CMap** movemap_pieces)
{
	PathfindingMap* p = new PathfindingMap(plane);
	if(p->Init(row, col, piece_indexes, pieces_count, movemap_pieces))
		return p;
	delete p;
	return NULL;
}

float GetWaterHeight(world * plane, float x, float z)
{
	A3DVECTOR3 pos(x,0,z);
	//return WaterAreaMap::GetWaterHeight(pos);
	return plane->GetMoveMap()->GetWaterHeight(pos);
}

bool IsValidSPPos(world * plane, const A3DVECTOR &pos)
{
	A3DVECTOR3 vPos(pos.x,pos.y,pos.z);
	//return CGlobalSPMap::GetInstance()->IsPosPassable(vPos);
	return plane->GetMoveMap()->IsPosPassable(vPos);
}

bool GetValidPos(world * plane, A3DVECTOR & pos)
{
	A3DVECTOR3 vPos(pos.x,pos.y,pos.z);
	if(plane->GetMoveMap()->GetGroundValid3DPos(vPos))
	{
		pos.y = vPos.y;
		return true;
	}
	else
	{
		return false;
	}
}

void GetKnockBackPos(world * plane, const A3DVECTOR & start, A3DVECTOR & end, int ienv)
{
	A3DVECTOR3 vStart(start.x,start.y,start.z);
	A3DVECTOR3 vEnd(end.x,end.y,end.z);
	A3DVECTOR3 vStopPos;
	switch(ienv)
	{
		case NPC_MOVE_ENV_ON_GROUND:
			plane->GetMoveMap()->GetGroundLastReachablePos(vStart,vEnd,vStopPos);
			end.x = vStopPos.x;
			end.y = vStopPos.y;
			end.z = vStopPos.z;
		break;
		case NPC_MOVE_ENV_ON_AIR:
			//CGlobalSPMap::GetInstance()->GetLastReachablePos(vStart,vEnd,vStopPos,CGlobalSPMap::Env_OnAir);
			plane->GetMoveMap()->GetLastReachablePos(vStart,vEnd,vStopPos,CGlobalSPMap::Env_OnAir);
			end.x = vStopPos.x;
			end.y = vStopPos.y;
			end.z = vStopPos.z;
		break;
		case NPC_MOVE_ENV_IN_WATER:
			//CGlobalSPMap::GetInstance()->GetLastReachablePos(vStart,vEnd,vStopPos,CGlobalSPMap::Env_UnderWater);
			plane->GetMoveMap()->GetLastReachablePos(vStart,vEnd,vStopPos,CGlobalSPMap::Env_UnderWater);
			end.x = vStopPos.x;
			end.y = vStopPos.y;
			end.z = vStopPos.z;
		break;
		default:
		//do nothing
		break;
	}
}
void 
keep_out::CreateAgent(world * plane, int iMoveEnv)
{
	_agent = CreateNPCFleeAgent(plane->GetMoveMap(), iMoveEnv);
}

void 
follow_target::CreateAgent(world * plane, int iMoveEnv, int Behavior)
{
	_agent = CreateNPCChaseAgent(plane->GetMoveMap(),iMoveEnv);
}

void cruise::CreateAgent(world * plane, int iMoveEnv)
{
	_agent = CreateNPCRambleAgent(plane->GetMoveMap(), iMoveEnv);
}

float PathfindingMap::GetTerrainHeight(float x, float z)
{ 
	return _plane->GetHeightAt(x,z); 
} 

}
