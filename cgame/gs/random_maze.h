#ifndef RANDOM_MAZE_H
#define RANDOM_MAZE_H

#include <vector>
#include <set>
#include <string>

namespace RandomMaze
{

enum Direction
{
	DirectionInvalid = -1,

	DirectionLeft  = 0, 
	DirectionUp    = 1, 
	DirectionRight = 2, 
	DirectionDown  = 3, 

	DirectionCount
};

class Room
{
public:
	static int InvalidRoomNo;

public:
	Room(int no);

	void SetRoomNo(int value) { m_roomNo = value; }
	int  GetRoomNo() const    { return m_roomNo; }
	bool CanWalkOut(Direction dir) const { return m_sides[dir]; }
	bool CanWalkOut() const { return m_sides[0] || m_sides[1] || m_sides[2] || m_sides[3]; }
	void SetSide(Direction dir, bool canWalkOut) { m_sides[dir] = canWalkOut; }
	void Enclose() { m_sides[0] = m_sides[1] = m_sides[2] = m_sides[3] = false; }
	int GetDoorCount() const;
	int GetWallCount() const;
	bool IsBranchEndRoom() const;
	bool IsEnclosedRoom() const;
	int GetDoors(std::vector<Direction>& doors) const;
	int GetWalls(std::vector<Direction>& walls) const;
	int GetDoorMask() const
	{
		int mask = 0;
		for(int i=0;i<(int)DirectionCount;i++)
		{
			if(m_sides[i])
				mask |= (1<<i);
		}
		return mask;
	}

private:
	int m_roomNo;
	bool m_sides[4];
}; // class Room

class MazeGen;

class Maze
{
	friend class MazeGen;

public:
	Maze(void);
	~Maze(void);
	Maze(const Maze & data);
	Maze& operator = (const Maze & rhs);

	bool Init(int horizRoomCount, int vertRoomCount, int entranceRoomNo, int exitRoomNo);
	void Clear();
	Room& GetRoom(int roomNo)             { return m_rooms[roomNo]; }
	const Room& GetRoom(int roomNo) const { return m_rooms[roomNo]; }
	int GetNextRoomNo(int roomNo, Direction dir) const;
	bool SetDoor(int room1, int room2);
	bool SetWall(int room1, int room2);
	bool IsValidRoom(int roomNo) const;
	bool IsNeighbor(int room1, int room2) const;
	bool IsEntranceRoom(int roomNo) const { return roomNo == m_entranceRoomNo; }
	bool IsExitRoom(int roomNo) const     { return roomNo == m_exitRoomNo; }
	void GetRoomCoord(int roomNo, int& x, int& y) const;
	int GetRoomNo(int x, int y) const;
	int GetManhDistance(int roomNo1, int roomNo2) const;

	int GetHorizRoomCount() const   { return m_horizRoomCount; }
	int GetVertRoomCount() const    { return m_vertRoomCount; }
	int GetTotalRoomCount() const   { return m_rooms.size(); }
	int GetEntranceRoomNo() const   { return m_entranceRoomNo; }
	int GetExitRoomNo() const       { return m_exitRoomNo; }
	int GetStepCount() const        { return m_stepCount; }
	int GetBranchCount() const      { return m_branchCount; }
	int GetBranchRoomCount() const  { return m_branchRoomCount; }
	int GetRingCount() const        { return m_ringCount; }

	void SetExitRoomNo(int roomNo) { m_exitRoomNo = roomNo; }
	void SetStepCount(int value)   { m_stepCount = value; }
	void SetBranchCount(int value) { m_branchCount = value; }
	void SetBranchRoomCount(int value)    { m_branchRoomCount = value; }
	void SetRingCount(int value)   { m_ringCount = value; }

	void IncStepCount()   { ++m_stepCount; }
	void DecStepCount()   { --m_stepCount; }
	void IncBranchCount() { ++m_branchCount; }
	void DecBranchCount() { --m_branchCount; }
	void IncBranchRoomCount() { ++m_branchRoomCount; }
	void DecBranchRoomCount() { --m_branchRoomCount; }
	void IncRingCount() { ++m_ringCount; }
	void DecRingCount() { --m_ringCount; }

protected:
	bool SetRelation(int room1, int room2, bool canRoom1ToRoom2, bool canRoom2ToRoom1);

protected:
	int m_horizRoomCount;
	int m_vertRoomCount;
	int m_entranceRoomNo;
	int m_exitRoomNo;
	int m_stepCount;
	int m_branchCount;
	int m_branchRoomCount;
	int m_ringCount;
	std::vector<Room> m_rooms;
}; // class Maze

class MazeGen
{
public:
	enum ErrorCode
	{
		ERR_PARAM_MAZE_SIZE    = 1,
		ERR_GEN_ENTRANCE_OR_EXIT,
		ERR_PARAM_ENTRANCE_OR_EXIT,
		ERR_PARAM_ENTRANCE_DOOR_DIRECTION,
		ERR_PARAM_EXIT_DOOR_DIRECTION,
		ERR_PARAM_STEP_COUNT,
		ERR_PARAM_BRANCH,
		ERR_PARAM_RING,
		ERR_VALID_ROOM_COUNT,
		ERR_MAZE_INIT,

		ERR_NOT_INITIALIZED,
		ERR_MAIN_PATH_LENGTH,
		ERR_BRANCH_COUNT, 
		ERR_BRANCH_ROOM_COUNT, 
		ERR_RING_COUNT,
	};

	enum State
	{
		MGS_INVALID            = 0,
		MGS_INITIALIZED        = 1,
		MGS_GEN_MAIN_PATH      = 2,
		MGS_GEN_BRANCH         = 3,
		MGS_ADJUST_BRANCH_ROOM = 4,
		MGS_GEN_RING           = 5,
		MGS_DONE               = 6,
	};

public:
	MazeGen();

	int Init(int horizRoomCount, int vertRoomCount, 
	         int entranceRoomNo, Direction entranceDoorDirection, 
	         int exitRoomNo, Direction exitDoorDirection,
	         int minStepCount, int maxStepCount,
	         int minBranchCount, int maxBranchCount, 
	         int minBranchRoomCount, int maxBranchRoomCount, 
	         int minRingCount, int maxRingCount);
	int Generate(bool showdetail = false);
	const int GetCurState() const { return m_state; }
	const Maze& GetMaze() const { return m_maze; }
	const std::vector<int>& GetMainPath() const { return m_mainPath; }
	void ShowRoom();

	static std::string TranslateErrorCode(int errorCode);

protected:
	static bool GenerateEntranceAndExitRoom(int& entranceRoomNo, int& exitRoomNo,
	                                        int entranceDoorDirection, int exitDoorDirection, 
                                            int horizRoomCount, int vertRoomCount, 
                                            int minStepCount, int maxStepCount);

	bool ConnectDirectly(int startRoomNo, int startRoomDoorDirection, int endRoomNo, int endRoomDoorDirection, std::vector<int>& visitedRooms);
	bool GenerateMainPath();
	bool AdjustBranch(int branchCount);
	bool AdjustBranchRoom();
	bool GenerateRing();
	void SetState(State state);

private:
	int m_entranceDoorDirection;
	int m_exitDoorDirection;
	int m_minStepCount;
	int m_maxStepCount;
	int m_minBranchCount;
	int m_maxBranchCount;
	int m_minBranchRoomCount;
	int m_maxBranchRoomCount;
	int m_minRingCount;
	int m_maxRingCount;

	Maze m_maze;

	int m_state;
	std::vector<int> m_mainPath; // rooms on main path
}; // class MazeGen

} // namespace RandomMaze
#endif
