#include "random_maze.h"
#include <ctime>
#include <stack>
#include <ASSERT.h>
#include <algorithm>
#include <map>
#include <cmath>

#define SHOW_DETAIL() if(showdetail) ShowRoom();
#define SHOW_TOKEN(token) if(showdetail) printf(token);

namespace RandomMaze
{

int Room::InvalidRoomNo = -1;

static bool IsValidDirection(int dir)
{
	return (dir >= DirectionLeft && dir <= DirectionDown);
}

static bool CheckRoomDoor(int roomNo, int doorDirection, int horizRoomCount, int vertRoomCount)
{
	if ((roomNo < horizRoomCount && DirectionUp == doorDirection) ||
	    (roomNo / horizRoomCount == vertRoomCount - 1 && DirectionDown == doorDirection) ||
	    (roomNo % horizRoomCount == 0 && DirectionLeft == doorDirection) ||
	    (roomNo % horizRoomCount == horizRoomCount - 1 && DirectionRight == doorDirection))
	{
		return false;
	}
	return true;
}

Room::Room(int no) : m_roomNo(no)
{
	m_sides[0] = m_sides[1] = m_sides[2] = m_sides[3] = false; // default enclose
}

int Room::GetDoorCount() const
{
	int doorCount = 0;
	for (int dir = DirectionLeft; dir < DirectionCount; ++dir)
	{
		if (m_sides[dir])
		{
			++doorCount;
		}
	}
	return doorCount;
}

int Room::GetWallCount() const
{
	return DirectionCount - GetDoorCount();
}

bool Room::IsBranchEndRoom() const
{
	return GetDoorCount() == 1;
}

bool Room::IsEnclosedRoom() const
{
	return GetDoorCount() == 0;
}

int Room::GetDoors(std::vector<Direction>& doors) const
{
	doors.clear();
	for (int dir = DirectionLeft; dir < DirectionCount; ++dir)
	{
		if (m_sides[dir])
		{
			doors.push_back(static_cast<Direction>(dir));
		}
	}
	return doors.size();
}

int Room::GetWalls(std::vector<Direction>& walls) const
{
	walls.clear();
	for (int dir = DirectionLeft; dir < DirectionCount; ++dir)
	{
		if (!m_sides[dir])
		{
			walls.push_back(static_cast<Direction>(dir));
		}
	}
	return walls.size();
}


Maze::Maze(void) 
	: m_horizRoomCount(0), m_vertRoomCount(0), m_entranceRoomNo(-1), m_exitRoomNo(-1), 
	  m_stepCount(0), m_branchCount(0), m_branchRoomCount(0), m_ringCount(0)
{
}

Maze::~Maze(void)
{
}

Maze::Maze(const Maze & data)
{
	m_horizRoomCount = data.m_horizRoomCount;
	m_vertRoomCount = data.m_vertRoomCount;
	m_entranceRoomNo = data.m_entranceRoomNo;
	m_exitRoomNo = data.m_exitRoomNo;
	m_stepCount = data.m_stepCount;
	m_branchCount = data.m_branchCount;
	m_branchRoomCount = data.m_branchRoomCount;
	m_ringCount = data.m_ringCount;
	m_rooms = data.m_rooms;
}

Maze& Maze::operator = (const Maze & rhs)
{
	if (this == &rhs) return *this;

	m_horizRoomCount = rhs.m_horizRoomCount;
	m_vertRoomCount = rhs.m_vertRoomCount;
	m_entranceRoomNo = rhs.m_entranceRoomNo;
	m_exitRoomNo = rhs.m_exitRoomNo;
	m_stepCount = rhs.m_stepCount;
	m_branchCount = rhs.m_branchCount;
	m_branchRoomCount = rhs.m_branchRoomCount;
	m_ringCount = rhs.m_ringCount;
	m_rooms = rhs.m_rooms;
	return *this;
}


bool Maze::Init(int horizRoomCount, int vertRoomCount, int entranceRoomNo, int exitRoomNo)
{
	ASSERT(horizRoomCount > 0 && vertRoomCount > 0 && 
	       entranceRoomNo >= 0 && entranceRoomNo < horizRoomCount * vertRoomCount &&
	       exitRoomNo >= 0 && exitRoomNo < horizRoomCount * vertRoomCount &&
	       entranceRoomNo != exitRoomNo);

	m_horizRoomCount = horizRoomCount;
	m_vertRoomCount = vertRoomCount;
	m_entranceRoomNo = entranceRoomNo;
	m_exitRoomNo = exitRoomNo;
	m_stepCount = 0;
	m_branchCount = 0;
	m_branchRoomCount = 0;
	m_ringCount = 0;

	int totalRoomCount = m_horizRoomCount * m_vertRoomCount;
	m_rooms.clear();
	m_rooms.reserve(totalRoomCount);
	for (int i = 0; i < totalRoomCount; ++i)
	{
		/*Room aRoom(i);
		if (i < m_horizRoomCount)
		{
			aRoom.SetSide(DirectionUp, false);
		}
		if (i + m_horizRoomCount >= totalRoomCount)
		{
			aRoom.SetSide(DirectionDown, false);
		}
		if ((i % m_horizRoomCount) == 0)
		{
			aRoom.SetSide(DirectionLeft, false);
		}
		if ((i % m_horizRoomCount) == m_horizRoomCount - 1)
		{
			aRoom.SetSide(DirectionRight, false);
		}*/
		m_rooms.push_back(Room(i));
	}
	return true;
}

void Maze::Clear()
{
	m_horizRoomCount = 0;
	m_vertRoomCount = 0;
	m_entranceRoomNo = 0;
	m_exitRoomNo = 0;
	m_stepCount = 0;
	m_branchCount = 0;
	m_branchRoomCount = 0;
	m_ringCount = 0;
	m_rooms.clear();
}

int Maze::GetNextRoomNo(int roomNo, Direction dir) const
{
	ASSERT(IsValidRoom(roomNo));

	int cur_x, cur_y;
	GetRoomCoord(roomNo, cur_x, cur_y);

	int nextRoomNo = -1;
	switch (dir)
	{
	case DirectionLeft:
		nextRoomNo = (0 == cur_x ? -1 : roomNo - 1);
		break;
	case DirectionUp:
		nextRoomNo = (0 == cur_y ? -1 : roomNo - m_horizRoomCount);
		break;
	case DirectionRight:
		nextRoomNo = (cur_x + 1 == m_horizRoomCount ? -1 : roomNo + 1);
		break;
	case DirectionDown:
		nextRoomNo = (cur_y + 1 == m_vertRoomCount ? -1 : roomNo + m_horizRoomCount);
		break;
	default:
		break;
	}
	return nextRoomNo;
}

bool Maze::SetRelation(int room1, int room2, bool canRoom1ToRoom2, bool canRoom2ToRoom1)
{
	if (room1 == room2 || !IsValidRoom(room1) || !IsValidRoom(room2))
	{
		return false;
	}

	int room1_x, room1_y, room2_x, room2_y;
	GetRoomCoord(room1, room1_x, room1_y);
	GetRoomCoord(room2, room2_x, room2_y);
	if (room1_x != room2_x && room1_y != room2_y)
	{
		return false;
	}

	if (room1_x + 1 == room2_x) // room2 is on the right side of room1
	{
		GetRoom(room1).SetSide(DirectionRight, canRoom1ToRoom2);
		GetRoom(room2).SetSide(DirectionLeft, canRoom2ToRoom1);
		return true;
	}
	else if (room1_x - 1 == room2_x) // room2 is on the left side of room1
	{
		GetRoom(room1).SetSide(DirectionLeft, canRoom1ToRoom2);
		GetRoom(room2).SetSide(DirectionRight, canRoom2ToRoom1);
		return true;
	}
	else if (room1_y + 1 == room2_y) // room2 is under room1
	{
		GetRoom(room1).SetSide(DirectionDown, canRoom1ToRoom2);
		GetRoom(room2).SetSide(DirectionUp, canRoom2ToRoom1);
		return true;
	}
	else if (room1_y - 1 == room2_y) // room2 is above room1
	{
		GetRoom(room1).SetSide(DirectionUp, canRoom1ToRoom2);
		GetRoom(room2).SetSide(DirectionDown, canRoom2ToRoom1);
		return true;
	}
	return false;
}

bool Maze::SetDoor(int room1, int room2)
{
	return SetRelation(room1, room2, true, true);
}

bool Maze::SetWall(int room1, int room2)
{
	return SetRelation(room1, room2, false, false);
}

bool Maze::IsValidRoom(int roomNo) const
{
	return (roomNo >= 0 && roomNo < GetTotalRoomCount());
}

bool Maze::IsNeighbor(int room1, int room2) const
{
	if (room1 == room2 || !IsValidRoom(room1) || !IsValidRoom(room2))
	{
		return false;
	}

	int room1_x, room1_y, room2_x, room2_y;
	GetRoomCoord(room1, room1_x, room1_y);
	GetRoomCoord(room2, room2_x, room2_y);
	return (abs(room2_x - room1_x) + abs(room2_y - room1_y) == 1);
}

void Maze::GetRoomCoord(int roomNo, int& x, int& y) const
{
	ASSERT(roomNo >= 0 && (size_t)roomNo < m_rooms.size());
	x = roomNo % m_horizRoomCount;
	y = roomNo / m_horizRoomCount;
}

int Maze::GetRoomNo(int x, int y) const
{
	if (x < 0 || x >= m_horizRoomCount || y < 0 || y >= m_vertRoomCount)
	{
		return -1;
	}
	return x + y * m_horizRoomCount;
}

int Maze::GetManhDistance(int roomNo1, int roomNo2) const
{
	if (!IsValidRoom(roomNo1) || !IsValidRoom(roomNo2))
	{
		return 0;
	}
	int room1_x, room1_y, room2_x, room2_y;
	GetRoomCoord(roomNo1, room1_x, room1_y);
	GetRoomCoord(roomNo2, room2_x, room2_y);
	return abs(room2_x - room1_x) + abs(room2_y - room1_y);
}

static int GetManhattanRooms(int room1, int room2, int horizRoomCount)
{
	ASSERT(room1 >= 0 && room2 >= 0);
	int x1 = room1 % horizRoomCount;
	int y1 = room1 / horizRoomCount;
	int x2 = room2 % horizRoomCount;
	int y2 = room2 / horizRoomCount;
	return abs(x2 - x1) + abs(y2 - y1) + 1;
}


struct Node
{
	int roomNo;
	Node* parent;
	std::map<int, Node> children;

	Node(Node* _parent = NULL, int _no = -1)
		: roomNo(_no), parent(_parent)
	{
	}
};

static int GetUnvisitedNeighborRooms(const Maze& maze, int curRoomNo, 
                                     const std::vector<int>& visitedRooms, 
									 const std::map<int, Node>* visitedTree, 
                                     std::vector<int>& unvisitedNeighborRooms)
{
	unvisitedNeighborRooms.clear();
	for (int dir = DirectionLeft; dir <= DirectionDown; ++dir)
	{
		int nextRoomNo = maze.GetNextRoomNo(curRoomNo, static_cast<Direction>(dir));
		if (maze.IsValidRoom(nextRoomNo) &&
			(NULL == visitedTree || visitedTree->find(nextRoomNo) == visitedTree->end()) && 
			std::find(visitedRooms.begin(), visitedRooms.end(), nextRoomNo) == visitedRooms.end()) // unvisited
		{
			unvisitedNeighborRooms.push_back(nextRoomNo);
		}
	}
	return unvisitedNeighborRooms.size();
}

static int GetUnvisitedNeighborCount(const Maze& maze, int curRoomNo)
{
	int count = 0;
	for (int dir = DirectionLeft; dir <= DirectionDown; ++dir)
	{
		int nextRoomNo = maze.GetNextRoomNo(curRoomNo, static_cast<Direction>(dir));
		if (maze.IsValidRoom(nextRoomNo) &&
			maze.GetRoom(nextRoomNo).IsEnclosedRoom()) // unvisited
		{
			++count;
		}
	}
	return count;
}


MazeGen::MazeGen()
	: m_entranceDoorDirection(DirectionInvalid), m_exitDoorDirection(DirectionInvalid), 
	  m_minStepCount(0), m_maxStepCount(0), m_minBranchCount(0), m_maxBranchCount(0), 
	  m_minBranchRoomCount(0), m_maxBranchRoomCount(0), m_minRingCount(0), m_maxRingCount(0), 
	  m_state(MGS_INVALID)
{
}


void MazeGen::SetState(State state)
{
	m_state = state;
}


bool MazeGen::GenerateEntranceAndExitRoom(int& entranceRoomNo, int& exitRoomNo,
                                          int entranceDoorDirection, int exitDoorDirection, 
                                          int horizRoomCount, int vertRoomCount, 
                                          int minStepCount, int maxStepCount)
{
	ASSERT(horizRoomCount > 0 && vertRoomCount > 0);
	ASSERT(minStepCount <= maxStepCount && minStepCount >= 0);
	ASSERT(entranceRoomNo < 0 || exitRoomNo < 0);

	int roomCount = horizRoomCount * vertRoomCount;
	if (minStepCount >= roomCount)
	{
		return false;
	}

	if ((1 == horizRoomCount && (DirectionLeft == entranceDoorDirection || 
	                             DirectionRight == entranceDoorDirection || 
	                             DirectionLeft == exitDoorDirection || 
	                             DirectionRight == exitDoorDirection)) || 
	    (1 == vertRoomCount && (DirectionUp == entranceDoorDirection || 
	                             DirectionDown == entranceDoorDirection || 
	                             DirectionUp == exitDoorDirection || 
	                             DirectionDown == exitDoorDirection)))
	{
		return false;
	}

	srand((unsigned int)time(NULL));
	int originalEntranceRoomNo = entranceRoomNo;
	int originalExitRoomNo = exitRoomNo;
	int retryTimes = 0;
	while (retryTimes++ < 200)
	{
		if (originalEntranceRoomNo < 0)
		{
			entranceRoomNo = (rand() % roomCount);
			if (!CheckRoomDoor(entranceRoomNo, entranceDoorDirection, horizRoomCount, vertRoomCount))
			{
				continue;
			}
		}
		if (originalExitRoomNo < 0)
		{
			exitRoomNo = (rand() % roomCount);
			if (!CheckRoomDoor(exitRoomNo, exitDoorDirection, horizRoomCount, vertRoomCount))
			{
				continue;
			}
		}
		int tmpManhDistance = GetManhattanRooms(entranceRoomNo, exitRoomNo, horizRoomCount);
		if (minStepCount > 2 && tmpManhDistance > 2 && tmpManhDistance < maxStepCount)
		{
			break;
		}
	}

	return true;
}


int MazeGen::Init(int horizRoomCount, int vertRoomCount, 
                  int entranceRoomNo, Direction entranceDoorDirection, 
                  int exitRoomNo, Direction exitDoorDirection, 
                  int minStepCount, int maxStepCount,
                  int minBranchCount, int maxBranchCount, 
                  int minBranchRoomCount, int maxBranchRoomCount, 
                  int minRingCount, int maxRingCount)
{
	SetState(MGS_INVALID);

	if (!IsValidDirection(entranceDoorDirection))
	{
		entranceDoorDirection = DirectionInvalid;
	}
	if (!IsValidDirection(exitDoorDirection))
	{
		exitDoorDirection = DirectionInvalid;
	}

	if (horizRoomCount <= 0 || vertRoomCount <= 0)
	{
		return ERR_PARAM_MAZE_SIZE;
	}
	if (entranceRoomNo < 0 || exitRoomNo < 0)
	{
		// Randomize entrance room and exit room
		if (!GenerateEntranceAndExitRoom(entranceRoomNo, exitRoomNo, 
		                                 entranceDoorDirection, exitDoorDirection, 
		                                 horizRoomCount, vertRoomCount, 
		                                 minStepCount, maxStepCount))
		{
			return ERR_GEN_ENTRANCE_OR_EXIT;
		}
	}
	if (entranceRoomNo < 0 || entranceRoomNo >= horizRoomCount * vertRoomCount || 
	    exitRoomNo < 0 || exitRoomNo >= horizRoomCount * vertRoomCount || 
	    entranceRoomNo == exitRoomNo)
	{
		return ERR_PARAM_ENTRANCE_OR_EXIT;
	}
	if (!CheckRoomDoor(entranceRoomNo, entranceDoorDirection, horizRoomCount, vertRoomCount))
	{
		return ERR_PARAM_ENTRANCE_DOOR_DIRECTION;
	}
	if (!CheckRoomDoor(exitRoomNo, exitDoorDirection, horizRoomCount, vertRoomCount))
	{
		return ERR_PARAM_EXIT_DOOR_DIRECTION;
	}
	if (minStepCount > maxStepCount || minStepCount <= 2 || 
	    maxStepCount < GetManhattanRooms(entranceRoomNo, exitRoomNo, horizRoomCount) || 
	    minStepCount > horizRoomCount * vertRoomCount)
	{
		return ERR_PARAM_STEP_COUNT;
	}
	if ((1 == horizRoomCount || 1 == vertRoomCount) && minStepCount > GetManhattanRooms(entranceRoomNo, exitRoomNo, horizRoomCount))
	{
		return ERR_PARAM_STEP_COUNT;
	}
	if (minBranchCount > maxBranchCount || minBranchCount < 0 || 
	    minBranchRoomCount > maxBranchRoomCount || minBranchRoomCount < 0 ||
	    minBranchCount > maxBranchRoomCount)
	{
		return ERR_PARAM_BRANCH;
	}
	if (minRingCount > maxRingCount || minRingCount < 0)
	{
		return ERR_PARAM_RING;
	}
	if (minStepCount + minBranchRoomCount > horizRoomCount * vertRoomCount)
	{
		return ERR_VALID_ROOM_COUNT;
	}

	if (!m_maze.Init(horizRoomCount, vertRoomCount, entranceRoomNo, exitRoomNo))
	{
		return ERR_MAZE_INIT;
	}
	if (m_maze.GetManhDistance(entranceRoomNo, exitRoomNo) == 1)
	{
		if (IsValidDirection(entranceDoorDirection) && IsValidDirection(exitDoorDirection) && 
			(m_maze.GetNextRoomNo(entranceRoomNo, entranceDoorDirection) != exitRoomNo || 
		     m_maze.GetNextRoomNo(exitRoomNo, exitDoorDirection) != entranceRoomNo))
		{
			return ERR_PARAM_ENTRANCE_OR_EXIT;
		}
	}
	// Isolate all rooms initially
/*	for (int i = 0; i < m_maze.GetTotalRoomCount(); ++i)
	{
		m_maze.GetRoom(i).Enclose();
	}*/ //defalut enclose now
	m_mainPath.clear();

	m_entranceDoorDirection = entranceDoorDirection;
	m_exitDoorDirection = exitDoorDirection;
	m_minStepCount = minStepCount;
	m_maxStepCount = maxStepCount;
	m_minBranchCount = minBranchCount;
	m_maxBranchCount = maxBranchCount;
	m_minBranchRoomCount = minBranchRoomCount;
	m_maxBranchRoomCount = maxBranchRoomCount;
	m_minRingCount = minRingCount;
	m_maxRingCount = maxRingCount;

	srand((unsigned int)time(NULL));
	SetState(MGS_INITIALIZED);
	return 0;
}


int MazeGen::Generate(bool showdetail)
{
	if (MGS_DONE == m_state)
	{
		return 0;
	}
	if (MGS_INITIALIZED != m_state)
	{
		return ERR_NOT_INITIALIZED;
	}

	SHOW_TOKEN("==============	GenerateMainPath	=================\n")
	// Try to satisfy step count condition
	if (!GenerateMainPath())
	{
		return ERR_MAIN_PATH_LENGTH;
	}

	SHOW_DETAIL();
	// Try to satisfy the branch count condition
	SHOW_TOKEN("==============	AdjustBranch	=================\n")
	int expectedBranchCount = m_minBranchCount + (rand() % (m_maxBranchCount - m_minBranchCount + 1));
	if (!AdjustBranch(expectedBranchCount))
	{
		return ERR_BRANCH_COUNT;
	}

	SHOW_DETAIL();
	// Try to satisfy branch room count condition
	while (m_maze.GetBranchCount() <= m_maxBranchCount)
	{
		SHOW_TOKEN("==============	AdjustBranchRoom	=================\n")
		if (AdjustBranchRoom())
		{
			SHOW_DETAIL();
			break;
		}

		if (m_maze.GetBranchCount() == m_maxBranchCount)
		{
			return ERR_BRANCH_ROOM_COUNT;
		}
		int deltaBranchCount = rand() % (m_maxBranchCount - m_maze.GetBranchCount() + 1);
		if (!AdjustBranch(m_maze.GetBranchCount() + deltaBranchCount))
		{
			return ERR_BRANCH_COUNT;
		}
		SHOW_DETAIL();
	}

	SHOW_TOKEN("==============	GenerateRing	=================\n")
	// Try to satisfy ring count condition
	if (!GenerateRing())
	{
		return ERR_RING_COUNT;
	}

	SHOW_DETAIL();
	SetState(MGS_DONE);
	return 0;
}


bool MazeGen::ConnectDirectly(int startRoomNo, int startRoomDoorDirection, int endRoomNo, int endRoomDoorDirection, std::vector<int>& visitedRooms)
{
	visitedRooms.clear();

	int tmpStartRoomNo = startRoomNo;
	int tmpEndRoomNo = endRoomNo;

	// If the entrance door direction or the exit door direction is specified 
	if (IsValidDirection(startRoomDoorDirection))
	{
		tmpStartRoomNo = m_maze.GetNextRoomNo(startRoomNo, (Direction)startRoomDoorDirection);
		if (!m_maze.IsValidRoom(tmpStartRoomNo))
		{
			return false;
		}

		m_maze.SetDoor(startRoomNo, tmpStartRoomNo);
		visitedRooms.push_back(startRoomNo);
		m_maze.IncStepCount();
	}
	if (IsValidDirection(endRoomDoorDirection))
	{
		tmpEndRoomNo = m_maze.GetNextRoomNo(endRoomNo, (Direction)endRoomDoorDirection);
		if (!m_maze.IsValidRoom(tmpEndRoomNo))
		{
			return false;
		}
	}

	int start_x, start_y, end_x, end_y;
	m_maze.GetRoomCoord(tmpStartRoomNo, start_x, start_y);
	m_maze.GetRoomCoord(tmpEndRoomNo, end_x, end_y);
	int cur_x = start_x;
	int cur_y = start_y;
	int step_x = (end_x - cur_x > 0 ? 1 : (end_x - cur_x == 0 ? 0 : -1));
	int step_y = (end_y - cur_y > 0 ? 1 : (end_y - cur_y == 0 ? 0 : -1));
	int curRoomNo = tmpStartRoomNo;
	int prevRoomNo = curRoomNo;
	visitedRooms.push_back(curRoomNo);
	m_maze.IncStepCount();
	while (curRoomNo != tmpEndRoomNo)
	{
		if (m_maze.GetManhDistance(curRoomNo, tmpEndRoomNo) > m_maze.GetManhDistance(startRoomNo, tmpEndRoomNo) || 
			m_maze.GetManhDistance(curRoomNo, tmpEndRoomNo) > m_maze.GetManhDistance(curRoomNo, endRoomNo))
		{
			std::vector<int> unvisitedNeighborRooms;
			GetUnvisitedNeighborRooms(m_maze, curRoomNo, visitedRooms, NULL, unvisitedNeighborRooms);
			if (tmpEndRoomNo != endRoomNo)
			{
				// Make sure endRoomNo won't be reached before tmpEndRoomNo 
				std::vector<int>::iterator it_EndRoom = std::find(unvisitedNeighborRooms.begin(), unvisitedNeighborRooms.end(), endRoomNo);
				if (unvisitedNeighborRooms.end() != it_EndRoom)
				{
					unvisitedNeighborRooms.erase(it_EndRoom);
				}
			}
			if (unvisitedNeighborRooms.empty())
			{
				return false;
			}
			int tmpMinManhDistance = 0x0FFFFFFF;
			curRoomNo = -1;
			for (std::vector<int>::iterator it = unvisitedNeighborRooms.begin(); it != unvisitedNeighborRooms.end(); ++it)
			{
				int tmpManhDistance = m_maze.GetManhDistance(*it, tmpEndRoomNo);
				if (tmpManhDistance < tmpMinManhDistance)
				{
					tmpMinManhDistance = tmpManhDistance;
					curRoomNo = *it;
				}
			}
			if (-1 == curRoomNo)
			{
				return false;
			}
			if (tmpEndRoomNo != endRoomNo && curRoomNo == endRoomNo)
			{
				continue;
			}
			m_maze.GetRoomCoord(curRoomNo, cur_x, cur_y);
		}
		else
		{
			int delta_x = 0, delta_y = 0;
			if (cur_x != end_x)
			{
				if (cur_y != end_y)
				{
					delta_x = ((rand() % 100) > 50 ? step_x : 0);
				}
				else
				{
					delta_x = step_x;
				}
			}
			if (cur_y != end_y)
			{
				if (cur_x != end_x && delta_x != 0)
				{
					delta_y = 0;
				}
				else
				{
					delta_y = step_y;
				}
			}
			curRoomNo = m_maze.GetRoomNo(cur_x + delta_x, cur_y + delta_y);
			if (std::find(visitedRooms.begin(), visitedRooms.end(), curRoomNo) != visitedRooms.end())
			{
				continue;
			}
			if (tmpEndRoomNo != endRoomNo && curRoomNo == endRoomNo)
			{
				continue;
			}
			cur_x += delta_x;
			cur_y += delta_y;
		}
		m_maze.SetDoor(prevRoomNo, curRoomNo);
		m_maze.IncStepCount();
		visitedRooms.push_back(curRoomNo);
		prevRoomNo = curRoomNo;
		// Adjust step
		step_x = (end_x - cur_x > 0 ? 1 : (end_x - cur_x == 0 ? 0 : -1));
		step_y = (end_y - cur_y > 0 ? 1 : (end_y - cur_y == 0 ? 0 : -1));
	}

	if (tmpEndRoomNo != endRoomNo)
	{
		m_maze.SetDoor(tmpEndRoomNo, endRoomNo);
		visitedRooms.push_back(endRoomNo);
		m_maze.IncStepCount();
	}
	return true;
}


bool MazeGen::GenerateMainPath()
{
	SetState(MGS_GEN_MAIN_PATH);

	int curRoomNo;

	// Generate the shortest path from entrance room to exit room
	if (!ConnectDirectly(m_maze.GetEntranceRoomNo(), m_entranceDoorDirection, 
	                     m_maze.GetExitRoomNo(), m_exitDoorDirection, 
	                     m_mainPath))
	{
		return false;
	}
	ASSERT((size_t)m_maze.GetStepCount() == m_mainPath.size());
	if (m_maze.GetStepCount() > m_maxStepCount)
	{
		return false;
	}
	else if (m_maze.GetStepCount() == m_maxStepCount)
	{
		return true;
	}
	if (m_maze.GetStepCount() <= 1)
	{
		return false;
	}

	if (m_maze.GetHorizRoomCount() > 1 && m_maze.GetVertRoomCount() > 1)
	{
		int retryTimes = 0;
		do
		{
			// Searching depth should be small enough to avoid time-consuming backtracking
			int expectedStepCount = 0;
			if (m_maze.GetStepCount() >= m_minStepCount)
			{
				expectedStepCount = m_minStepCount + (rand() % ((m_maxStepCount - m_minStepCount + 1) / 3));
				if (m_maze.GetStepCount() >= expectedStepCount)
				{
					break;
				}
			}
			else
			{
				expectedStepCount = m_maze.GetStepCount() + (rand() % 6);
				if (expectedStepCount > m_maxStepCount)
				{
					expectedStepCount = m_maxStepCount;
				}
				if (m_maze.GetStepCount() >= expectedStepCount)
				{
					continue;
				}
			}

			ASSERT(m_mainPath.size() > 0);
			int roomNo1, roomNo2;
			if (m_mainPath.size() > 2)
			{
				int index1 = (rand() % m_mainPath.size());
				roomNo1 = m_mainPath[index1];
				if ((m_maze.GetEntranceRoomNo() == roomNo1 && IsValidDirection(m_entranceDoorDirection)) || 
					(m_maze.GetExitRoomNo() == roomNo1 && IsValidDirection(m_exitDoorDirection)))
				{
					// If the entrance door direction or the exit door direction is specified
					continue;
				}
				Room& room1 = m_maze.GetRoom(roomNo1);
				std::vector<Direction> doors;
				room1.GetDoors(doors);
				ASSERT(doors.size() > 0);
				roomNo2 = m_maze.GetNextRoomNo(roomNo1, doors[rand() % doors.size()]);
				if ((m_maze.GetEntranceRoomNo() == roomNo2 && IsValidDirection(m_entranceDoorDirection)) || 
					(m_maze.GetExitRoomNo() == roomNo2 && IsValidDirection(m_exitDoorDirection)))
				{
					// If the entrance door direction or the exit door direction is specified
					continue;
				}
				m_maze.SetWall(roomNo1, roomNo2);
				doors.clear();
				if (m_maze.GetRoom(roomNo2).GetDoors(doors) == 0)
				{
					int tmpRoomNo = roomNo1;
					roomNo1 = roomNo2;
					roomNo2 = tmpRoomNo;
					m_maze.GetRoom(roomNo2).GetDoors(doors);
					ASSERT(doors.size() == 1);
				}
				int roomNo3 = m_maze.GetNextRoomNo(roomNo2, doors[rand() % doors.size()]);
				if ((m_maze.GetEntranceRoomNo() == roomNo3 && IsValidDirection(m_entranceDoorDirection)) || 
					(m_maze.GetExitRoomNo() == roomNo3 && IsValidDirection(m_exitDoorDirection)))
				{
					// If the entrance door direction or the exit door direction is specified
					//m_maze.SetDoor(roomNo1, roomNo2);
					//continue;
				}
				else
				{
					m_maze.SetWall(roomNo2, roomNo3);
					m_mainPath.erase(std::find(m_mainPath.begin(), m_mainPath.end(), roomNo2));
					m_maze.DecStepCount();
					roomNo2 = roomNo3;
				}
			}
			else
			{
				roomNo1 = m_mainPath[0];
				roomNo2 = m_mainPath[1];
				if (((m_maze.GetEntranceRoomNo() == roomNo1 || m_maze.GetEntranceRoomNo() == roomNo2) && IsValidDirection(m_entranceDoorDirection)) ||  
					((m_maze.GetExitRoomNo() == roomNo1 || m_maze.GetExitRoomNo() == roomNo2) && IsValidDirection(m_exitDoorDirection)))
				{
					// If the entrance door direction or the exit door direction is specified
					break;
				}
				m_maze.SetWall(roomNo1, roomNo2);
				m_maze.DecStepCount();
				m_maze.DecStepCount();
			}

			// Make sure room1 is ahead of room2
			if (std::find(m_mainPath.begin(), m_mainPath.end(), roomNo1) > std::find(m_mainPath.begin(), m_mainPath.end(), roomNo2))
			{
				int tmpRoomNo = roomNo1;
				roomNo1 = roomNo2;
				roomNo2 = tmpRoomNo;
			}

			std::stack<int> wayPointStack;
			std::vector<int> unvisitedNeighborRooms;
			Node visitedTree;
			curRoomNo = roomNo1;
			wayPointStack.push(curRoomNo);
			Node* curNode = & visitedTree;
			curNode->roomNo = curRoomNo;
			curNode->parent = NULL;
			while (curRoomNo != roomNo2)
			{
				unvisitedNeighborRooms.clear();

				if (m_maze.GetStepCount() + GetManhattanRooms(curRoomNo, roomNo2, m_maze.GetHorizRoomCount()) - 2 <= expectedStepCount)
				{
					GetUnvisitedNeighborRooms(m_maze, curRoomNo, m_mainPath, &(curNode->children), unvisitedNeighborRooms);
					if (m_maze.IsNeighbor(curRoomNo, roomNo2))
					{
						unvisitedNeighborRooms.push_back(roomNo2);
					}
				}

				if (unvisitedNeighborRooms.size() > 0)
				{
					int nextRoomNo = unvisitedNeighborRooms[rand() % unvisitedNeighborRooms.size()];
					m_maze.SetDoor(curRoomNo, nextRoomNo);
					if (nextRoomNo != roomNo2)
					{
						m_maze.IncStepCount();
						m_mainPath.insert(std::find(m_mainPath.begin(), m_mainPath.end(), roomNo2), nextRoomNo);
					}
					curNode->children[nextRoomNo] = Node(curNode, nextRoomNo);
					wayPointStack.push(nextRoomNo);
					curRoomNo = nextRoomNo;
					curNode = &curNode->children[curRoomNo];
					continue;
				}
				else if (wayPointStack.size() > 0)
				{
					wayPointStack.pop();
				}

				if (wayPointStack.size() > 0)
				{
					m_maze.SetWall(wayPointStack.top(), curRoomNo);
					m_maze.DecStepCount();
					m_mainPath.erase(std::find(m_mainPath.begin(), m_mainPath.end(), curRoomNo));
					curRoomNo = wayPointStack.top();
					curNode = curNode->parent;
				}
				else
				{
					// ATTENTION: NEVER REACH HERE
					break;
				}
			}
		} while (m_maze.GetStepCount() < m_minStepCount && ++retryTimes <= 500);
	}

	return (m_maze.GetStepCount() >= m_minStepCount && m_maze.GetStepCount() <= m_maxStepCount);
}


bool MazeGen::AdjustBranch(int branchCount)
{
	SetState(MGS_GEN_BRANCH);

	if (m_maze.GetHorizRoomCount() > 1 && m_maze.GetVertRoomCount() > 1)
	{
		// Try to satisfy the branch count condition
		if (m_maze.GetBranchCount() < branchCount)
		{
			std::vector<int> candidateRooms;
			for (int roomNo = 0; roomNo < m_maze.GetTotalRoomCount(); ++roomNo)
			{
				Room& room = m_maze.GetRoom(roomNo);
				if (!room.IsEnclosedRoom() && !room.IsBranchEndRoom())
				{
					if ((m_maze.GetEntranceRoomNo() == roomNo && IsValidDirection(m_entranceDoorDirection)) || 
						(m_maze.GetExitRoomNo() == roomNo && IsValidDirection(m_exitDoorDirection)))
					{
						// If the entrance door direction or the exit door direction is specified
						continue;
					}
					candidateRooms.push_back(roomNo);
				}
			}
			while (m_maze.GetBranchCount() < branchCount && candidateRooms.size() > 0)
			{
				int index = (rand() % candidateRooms.size());
				int roomNo = candidateRooms[index];
				std::vector<Direction> walls;
				m_maze.GetRoom(roomNo).GetWalls(walls);
				while (walls.size() > 0)
				{
					int index = (rand() % walls.size());
					int nextRoomNo = m_maze.GetNextRoomNo(roomNo, walls[index]);
					if (m_maze.IsValidRoom(nextRoomNo) && m_maze.GetRoom(nextRoomNo).IsEnclosedRoom())
					{
						m_maze.SetDoor(roomNo, nextRoomNo);
						m_maze.IncBranchCount();
						m_maze.IncBranchRoomCount();
						break;
					}
					else
					{
						walls.erase(walls.begin() + index);
					}
				}
				if (walls.size() == 0)
				{
					candidateRooms.erase(candidateRooms.begin() + index);
				}
			}
		}
		else if (m_maze.GetBranchCount() > branchCount)
		{
			std::vector<int> branchEndRooms;
			for (int roomNo = 0; roomNo < m_maze.GetTotalRoomCount() && branchEndRooms.size() < (size_t)m_maze.GetBranchCount(); ++roomNo)
			{
				if (m_maze.GetRoom(roomNo).IsBranchEndRoom() && !m_maze.IsEntranceRoom(roomNo) && !m_maze.IsExitRoom(roomNo))
				{
					branchEndRooms.push_back(roomNo);
				}
			}

			while (m_maze.GetBranchCount() > branchCount && branchEndRooms.size() > 0)
			{
				int index = (rand() % branchEndRooms.size());
				int branchEndRoomNo = branchEndRooms[index];
				std::vector<Direction> doors;
				m_maze.GetRoom(branchEndRoomNo).GetDoors(doors);
				ASSERT(doors.size() == 1);
				int nextRoomNo = m_maze.GetNextRoomNo(branchEndRoomNo, doors[0]);
				if (m_maze.IsValidRoom(nextRoomNo))
				{
					m_maze.SetWall(nextRoomNo, branchEndRoomNo);
					m_maze.DecBranchRoomCount();
					branchEndRooms.erase(branchEndRooms.begin() + index);
					if (m_maze.GetRoom(nextRoomNo).IsBranchEndRoom())
					{
						branchEndRooms.push_back(nextRoomNo);
					}
					else
					{
						m_maze.DecBranchCount();
					}
				}
			}
		}
	}

	return (m_maze.GetBranchCount() >= m_minBranchCount && m_maze.GetBranchCount() <= m_maxBranchCount);
}

bool MazeGen::AdjustBranchRoom()
{
	SetState(MGS_ADJUST_BRANCH_ROOM);

	if (m_maze.GetBranchRoomCount() >= m_minBranchRoomCount && m_maze.GetBranchRoomCount() <= m_maxBranchRoomCount)
	{
		return true;
	}

	if (m_maze.GetHorizRoomCount() > 1 && m_maze.GetVertRoomCount() > 1)
	{
		std::vector<int> branchEndRooms;
		for (int roomNo = 0; roomNo < m_maze.GetTotalRoomCount() && branchEndRooms.size() < (size_t)m_maze.GetBranchCount(); ++roomNo)
		{
			if (m_maze.GetRoom(roomNo).IsBranchEndRoom() && !m_maze.IsEntranceRoom(roomNo) && !m_maze.IsExitRoom(roomNo))
			{
				std::vector<Direction> walls;
				m_maze.GetRoom(roomNo).GetWalls(walls);
				for (std::vector<Direction>::iterator it = walls.begin(); it != walls.end(); ++it)
				{
					int nextRoomNo = m_maze.GetNextRoomNo(roomNo, *it);
					if (m_maze.IsValidRoom(nextRoomNo) && m_maze.GetRoom(nextRoomNo).IsEnclosedRoom())
					{
						branchEndRooms.push_back(roomNo);
						break;
					}
				}
			}
		}

		// Try to satisfy branch room count condition
		// TODO: How if all branch end rooms have no unvisited neighbors?
		if (m_maze.GetBranchRoomCount() < m_minBranchRoomCount)
		{
			while (m_maze.GetBranchRoomCount() < m_minBranchRoomCount && branchEndRooms.size() > 0)
			{
				std::vector<Direction> walls;
				int index = (rand() % branchEndRooms.size());
				int nextRoomNo = 0;
				int branchEndRoomNo = branchEndRooms[index];
				ASSERT(branchEndRoomNo >= 0);
				m_maze.GetRoom(branchEndRoomNo).GetWalls(walls);
				while (walls.size() > 0)
				{
					int randWallIndex = (rand() % walls.size());
					nextRoomNo = m_maze.GetNextRoomNo(branchEndRoomNo, walls[randWallIndex]);
					if (m_maze.IsValidRoom(nextRoomNo) && m_maze.GetRoom(nextRoomNo).IsEnclosedRoom())
					{
						m_maze.SetDoor(branchEndRoomNo, nextRoomNo);
						m_maze.IncBranchRoomCount();

						branchEndRooms.erase(branchEndRooms.begin() + index);
						branchEndRooms.push_back(nextRoomNo);
						break;
					}
					else
					{
						walls.erase(walls.begin() + randWallIndex);
					}
				}
				if (walls.size() == 0)
				{
					branchEndRooms.erase(branchEndRooms.begin() + index);
				}
			}
		}
		else if (m_maze.GetBranchRoomCount() > m_maxBranchRoomCount)
		{
			while (m_maze.GetBranchRoomCount() > m_maxBranchRoomCount && branchEndRooms.size() > 0)
			{
				std::vector<Direction> doors;
				int index = (rand() % branchEndRooms.size());
				int nextRoomNo = 0;
				int branchEndRoomNo = branchEndRooms[index];
				ASSERT(branchEndRoomNo >= 0);
				m_maze.GetRoom(branchEndRoomNo).GetDoors(doors);
				while (doors.size() > 0)
				{
					int randDoorIndex = (rand() % doors.size());
					nextRoomNo = m_maze.GetNextRoomNo(branchEndRoomNo, doors[randDoorIndex]);
					if (m_maze.IsValidRoom(nextRoomNo) && m_maze.GetRoom(nextRoomNo).GetDoorCount() == 2)
					{
						m_maze.SetWall(nextRoomNo, branchEndRoomNo);
						m_maze.DecBranchRoomCount();

						branchEndRooms.erase(branchEndRooms.begin() + index);
						branchEndRooms.push_back(nextRoomNo);
						break;
					}
					else
					{
						doors.erase(doors.begin() + randDoorIndex);
					}
				}
				if (doors.size() == 0)
				{
					branchEndRooms.erase(branchEndRooms.begin() + index);
				}
			}
		}
	}

	return (m_maze.GetBranchRoomCount() >= m_minBranchRoomCount && m_maze.GetBranchRoomCount() <= m_maxBranchRoomCount);
}


bool MazeGen::GenerateRing()
{
	ASSERT(m_mainPath.size() >= 2);

	SetState(MGS_GEN_RING);

	if (m_maze.GetHorizRoomCount() > 1 && m_maze.GetVertRoomCount() > 1)
	{
		int startRoomNo = -1, endRoomNo = -1, curRoomNo = -1;
		int distanceOnMainPath = 0;
		int retryTimes = 0;
		while (m_maze.GetRingCount() < m_minRingCount && ++retryTimes <= 50)
		{
			int retryTimes2 = 0;
			while (++retryTimes2 <= 50)
			{
				int startIndex = (rand() % m_mainPath.size());
				int endIndex = (rand() % m_mainPath.size());
				distanceOnMainPath = abs(endIndex - startIndex) + 1;
				if(distanceOnMainPath >= 6) continue;
				startRoomNo = m_mainPath[startIndex];
				endRoomNo = m_mainPath[endIndex];
				if (((m_maze.GetEntranceRoomNo() == startRoomNo || m_maze.GetEntranceRoomNo() == endRoomNo) && IsValidDirection(m_entranceDoorDirection)) || 
					((m_maze.GetExitRoomNo() == startRoomNo || m_maze.GetExitRoomNo() == endRoomNo) && IsValidDirection(m_exitDoorDirection)))
				{
					// If the entrance door direction or the exit door direction is specified
					continue;
				}
				if (endIndex != startIndex && 
					GetUnvisitedNeighborCount(m_maze, startRoomNo) > 0 && 
					GetUnvisitedNeighborCount(m_maze, endRoomNo) > 0) 
				{
					break;
				}
			}
			if (retryTimes2 > 50)
			{
				continue;
			}

			std::vector<int> visitedRooms;
			std::stack<int> wayPointStack;
			std::vector<int> unvisitedNeighborRooms;
			Node visitedTree;
			int steps = 1;
			int originalVisitedRoomCount = 0;

			for (int roomNo = 0; roomNo < m_maze.GetTotalRoomCount(); ++roomNo)
			{
				if (!m_maze.GetRoom(roomNo).IsEnclosedRoom())
				{
					visitedRooms.push_back(roomNo);
				}
			}
			originalVisitedRoomCount = visitedRooms.size();
			curRoomNo = startRoomNo;
			wayPointStack.push(curRoomNo);
			Node* curNode = &visitedTree;
			curNode->roomNo = curRoomNo;
			curNode->parent = NULL;
			while (curRoomNo != endRoomNo && wayPointStack.size() > 0)
			{
				unvisitedNeighborRooms.clear();
				if (visitedRooms.size() < (size_t)originalVisitedRoomCount + 8)
				{
					GetUnvisitedNeighborRooms(m_maze, curRoomNo, visitedRooms, &(curNode->children), unvisitedNeighborRooms);
					if (m_maze.IsNeighbor(curRoomNo, endRoomNo))
					{
						if (steps >= distanceOnMainPath) // make sure ring won't be a shortcut to mainpath
						{
							unvisitedNeighborRooms.clear();
							unvisitedNeighborRooms.push_back(endRoomNo);
						}
					}
				}

				if (unvisitedNeighborRooms.size() > 0)
				{
					int nextRoomNo = unvisitedNeighborRooms[rand() % unvisitedNeighborRooms.size()];
					m_maze.SetDoor(curRoomNo, nextRoomNo);
					++steps;
					if (nextRoomNo != endRoomNo)
					{
						visitedRooms.push_back(nextRoomNo);
					}
			//		if (nextRoomNo != endRoomNo || steps >= distanceOnMainPath - 1) //unvisitedNeighborRooms already exclude shortcut
					{
						curNode->children[nextRoomNo] = Node(curNode, nextRoomNo);
						wayPointStack.push(nextRoomNo);
						curRoomNo = nextRoomNo;
						curNode = &curNode->children[curRoomNo];
						continue;
					}
				}

				// 回溯
				if (wayPointStack.size() > 0)
				{
					wayPointStack.pop();
				}

				if (wayPointStack.size() > 0)
				{
					m_maze.SetWall(wayPointStack.top(), curRoomNo);
					--steps;
					ASSERT(visitedRooms.back() == curRoomNo);
					visitedRooms.erase(visitedRooms.end() - 1);
					curRoomNo = wayPointStack.top();
					curNode = curNode->parent;
				}
				else
				{
					// ATTENTION: NEVER REACH HERE
					break;
				}
			}
			if (curRoomNo == endRoomNo && steps > distanceOnMainPath)
			{
				m_maze.IncRingCount();
			}
		}
	}

	return (m_maze.GetRingCount() >= m_minRingCount && m_maze.GetRingCount() <= m_maxRingCount);
}


std::string MazeGen::TranslateErrorCode(int errorCode)
{
	switch (errorCode)
	{
	case 0:
		return "成功";
	case MazeGen::ERR_PARAM_MAZE_SIZE:
		return "迷宫大小设定错误";
	case MazeGen::ERR_GEN_ENTRANCE_OR_EXIT:
		return "生成入口/出口错误";
	case MazeGen::ERR_PARAM_ENTRANCE_OR_EXIT:
		return "入口/出口设定错误";
	case MazeGen::ERR_PARAM_ENTRANCE_DOOR_DIRECTION:
		return "入口门位置设定错误";
	case MazeGen::ERR_PARAM_EXIT_DOOR_DIRECTION:
		return "出口门位置设定错误";
	case MazeGen::ERR_PARAM_STEP_COUNT:
		return "主路径总房间数设定错误";
	case MazeGen::ERR_PARAM_BRANCH:
		return "分支设定错误";
	case MazeGen::ERR_PARAM_RING:
		return "环数设定错误";
	case MazeGen::ERR_VALID_ROOM_COUNT:
		return "最小有效房间数设定错误";
	case MazeGen::ERR_MAZE_INIT:
		return "迷宫初始化失败";
	case MazeGen::ERR_NOT_INITIALIZED:
		return "迷宫生成器未初始化";
	case MazeGen::ERR_MAIN_PATH_LENGTH:
		return "主路径长度错误";
	case MazeGen::ERR_BRANCH_COUNT:
		return "分支数错误";
	case MazeGen::ERR_BRANCH_ROOM_COUNT:
		return "分支房间数错误";
	case MazeGen::ERR_RING_COUNT:
		return "环路径数错误";
	default:
		return "未知错误";
	}
	return "";
}

void MazeGen::ShowRoom()
{
	printf("[RandomMap Gen]Show Room\n");
	
	int x_blocks = m_maze.GetHorizRoomCount();
	int y_blocks = m_maze.GetVertRoomCount();
	
	for(int i = 0;i < y_blocks;++i)
	{
		for(int j = 0;j < x_blocks;++j)
		{
			int index = x_blocks * i + j;
			const RandomMaze::Room& room = m_maze.GetRoom(index);

			if(index == m_maze.GetEntranceRoomNo())
			{				
				printf("①");
			}
			else if(index == m_maze.GetExitRoomNo())
			{
				printf("②");				
			}
			else
			{
				int mask = room.GetDoorMask();
				switch(mask)
				{
					case 0:
						printf("  ");
						break;
					case 1:	
						printf("┅");
						break;
					case 2:	
						printf("┇");
						break;
					case 3:	
						printf("┛");
						break;
					case 4:	
						printf("┅");
						break;
					case 5:	
						printf("━");
						break;
					case 6:	
						printf("┗");
						break;
					case 7:	
						printf("┻");
						break;
					case 8:	
						printf("┇");
						break;
					case 9:	
						printf("┓");
						break;
					case 10:
						printf("┃");
						break;
					case 11:	
						printf("┫");
						break;
					case 12:	
						printf("┏");
						break;
					case 13:	
						printf("┳");
						break;
					case 14:	
						printf("┣");
						break;
					case 15:	
						printf("╋");
						break;
				}
			}
		}
		printf("\n");
	}
}

} // namespace RandomMaze

