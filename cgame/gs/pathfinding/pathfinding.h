#ifndef __ONLINE_GAME_GS_PATHFINDING_H__
#define __ONLINE_GAME_GS_PATHFINDING_H__

#include "NPCMove.h"
#include "NPCMoveMap.h"
#include "NPCChaseOnGroundAgent.h"
#include "NPCMoveAgent.h"
#include "GlobalSPMap.h"
#include "CMap.h"
#include "../substance.h"
#include <common/types.h>


class world;

namespace path_finding
{
	NPCMoveMap::CMap* InitMoveMap(const char* move_map_path, const char* water_map_path, const char* air_map, world * plane);
	NPCMoveMap::CMap* InitMoveMapPiece(const char* move_map_path, const char* water_map_path, const char* air_map, world * plane, int piece_idx);
	NPCMoveMap::CMap* InitMoveMap(world * plane, int row, int col, const int * piece_indexes, size_t pieces_count, NPCMoveMap::CMap** movemap_pieces);
	bool GetValidPos(world * plane, A3DVECTOR & pos);	//传入地面上的坐标点
	bool IsValidSPPos(world * plane, const A3DVECTOR & pos);
	void GetKnockBackPos(world * plane, const A3DVECTOR & start, A3DVECTOR & end,int ienv);
	float GetWaterHeight(world * plane, float x, float z);

	typedef CChaseInfo chase_info;
	class follow_target: public substance 
	{
		protected:
			CNPCChaseAgent* _agent;
			A3DVECTOR _target;
			short _step_counter;
			char _detail_level;
			char _step_level;
			static int _detail_param[3][4];
		public:
			follow_target():_agent(0)
			{}

			~follow_target()
			{
				if(_agent) 
				{
					_agent->Release();
					delete _agent;
				}
			}

			void CreateAgent(world * plane, int iMoveEnv, int Behavior);

			void Start(const A3DVECTOR & source, const A3DVECTOR & target, float speed, float range,float cur_distance, chase_info * pInfo)
			{
				if(cur_distance <= 100.f)
					_detail_level = 0;
				else if(cur_distance <= 400.f)
					_detail_level = 1;
				else 
					_detail_level = 2;
				_step_counter = 0;
				_step_level = 0;
				_target = target;

				A3DVECTOR3 initPos(source.x,source.y,source.z);
				_agent->Init(initPos, speed);           // 当前位置

				_agent->SetGoal(A3DVECTOR3(target.x,target.y,target.z), range,pInfo);
				_agent ->StartPathFinding(_detail_param[(int)_detail_level][1]);
			}

			bool GetToGoal()
			{
				return _agent ->GetToGoal();
			}

			bool PathIsFound()
			{
				return 	_agent->FoundPath();
			}

			void SetSteps(int steps)
			{
				_agent->SetPFPixels(steps);
			}

			void GetCurPos(A3DVECTOR & pos)
			{
				const A3DVECTOR3 & cp = _agent->GetCurPos();
				pos.x = cp.x;
				pos.y = cp.y;
				pos.z = cp.z;
			}

			bool MoveOneStep(float speed)
			{
				_agent->SetMoveStep(speed);
				_agent->MoveOneStep();
				_step_counter += _agent->GetPFPixels();
				if(IsBlocked()) 
				{
					if(!_step_level)
					{
						_step_level = 1;
						_agent->SetPFPixels(_detail_param[(int)_detail_level][2]);
					}
				}
				else
				{
					_step_level = 0;
					_agent->SetPFPixels(_detail_param[(int)_detail_level][1]);
				}
				if(IsFailed()) return false;
				return PathIsFound() || _step_counter < _detail_param[(int)_detail_level][0];
			}

			bool IsBlocked()
			{
				return _agent->IsBlocked();
			}

			bool IsFailed()
			{
				return _agent->BlockedBeyondEnv();
			}

			bool IsGoalReachable()
			{
				return _agent->IsGoalReachable();
			}

			const A3DVECTOR & GetTarget()
			{
				return _target;
			}
	};

	class cruise : public substance 
	{
		protected:
			CNPCRambleAgent * _agent;
		public:
			cruise():_agent(0)
			{}

			~cruise()
			{
				if(_agent) 
				{
					_agent->Release();
					delete _agent;
				}
			}

			void CreateAgent(world * plane, int iMoveEnv);

			void Start(const A3DVECTOR & source, const A3DVECTOR & center, float speed, float range)
			{

				A3DVECTOR3 initPos(source.x,source.y,source.z);
				A3DVECTOR3 cenPos(center.x,center.y,center.z);
				_agent->Init(initPos, speed);           // 当前位置
				_agent->SetRambleRange(cenPos, range);

				_agent->StartRamble();
			}

			bool GetToGoal()
			{
				return _agent ->Stopped();
			}

			void GetCurPos(A3DVECTOR & pos)
			{
				const A3DVECTOR3 & cp = _agent->GetCurPos();
				pos.x = cp.x;
				pos.y = cp.y;
				pos.z = cp.z;
			}

			bool MoveOneStep(float speed)
			{
				_agent->MoveOneStep();
				return true;
			}
	};

	class keep_out : public substance
	{
		protected:
			CNPCFleeAgent * _agent;
		public:
			keep_out():_agent(0)
			{}

			~keep_out()
			{
				if(_agent) 
				{
					_agent->Release();
					delete _agent;
				}
			}

			void CreateAgent(world * plane, int iMoveEnv);

			void Start(const A3DVECTOR & source, const A3DVECTOR & center, float speed, float range)
			{

				A3DVECTOR3 initPos(source.x,source.y,source.z);
				A3DVECTOR3 cenPos(center.x,center.y,center.z);
				_agent->Init(initPos, speed);           // 当前位置
				_agent->SetFleePos(cenPos, range);
				_agent->StartFlee();
			}

			bool GetToGoal()
			{
				return _agent ->FleeSuccess();
			}

			void GetCurPos(A3DVECTOR & pos)
			{
				const A3DVECTOR3 & cp = _agent->GetCurPos();
				pos.x = cp.x;
				pos.y = cp.y;
				pos.z = cp.z;
			}

			void SetFleePos(const A3DVECTOR & target, float range)
			{
				A3DVECTOR3 fleePos(target.x,target.y,target.z);
				_agent->SetFleePos(fleePos, range);
			}
			
			bool MoveOneStep(float speed)
			{
				_agent->MoveOneStep();
				return true;
			}
	};

	class PathfindingMap : public NPCMoveMap::CMap
	{
		world * _plane;
	public:
		PathfindingMap(world * plane):_plane(plane){}
		float GetTerrainHeight(float x, float z);
	};
}

#endif

