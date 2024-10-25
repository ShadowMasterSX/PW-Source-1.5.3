#ifndef __VCLIENT_MOVEAGENT_H__
#define __VCLIENT_MOVEAGENT_H__

#include <stdlib.h>
#include <vector>
#include "types.h"
#include "ASSERT.h"
#include "spinlock.h"

#define MOVEAGENT_ROW		4000
#define MOVEAGENT_COL		4000
#define MOVEAGENT_STEP 		1.f
#define MOVEAGENT_X_START	-2000.f
#define MOVEAGENT_Z_START	-2000.f
#define MOVEAGENT_SEARCH_RANGE 5

unsigned char DirConvert(const A3DVECTOR & dir);	//·µ»ØÖµ0-255
class MoveAgent
{
public:
	void Init();
	void Quit();
	void Learn(const A3DVECTOR & rpos);
	bool GetMovePos(const A3DVECTOR & cur_pos, const A3DVECTOR & dest_pos, float range, A3DVECTOR & next_pos);
	bool GetMovePos(const A3DVECTOR & cur_pos, unsigned char dir, float range, A3DVECTOR & next_pos);

	MoveAgent()
	{
		reachable_table = new A3DVECTOR **[MOVEAGENT_ROW];
		for(int i=0; i<MOVEAGENT_ROW; i++)
			reachable_table[i] = new A3DVECTOR *[MOVEAGENT_COL];

		for(int i=0; i<MOVEAGENT_ROW; i++)
			for(int j=0; j<MOVEAGENT_COL; j++)
				reachable_table[i][j] = NULL;

		for(int i=1; i<=MOVEAGENT_SEARCH_RANGE; i++)
			for(int j=0; j<=i; j++)
			{
				off_node_list[0].push_back(off_node_t(i,j));
				off_node_list[7].push_back(off_node_t(i,-j));
				off_node_list[3].push_back(off_node_t(-i,j));
				off_node_list[4].push_back(off_node_t(-i,-j));
			}
		for(int j=1; j<=MOVEAGENT_SEARCH_RANGE; j++)
			for(int i=0; i<=j; i++)
			{
				off_node_list[1].push_back(off_node_t(i,j));
				off_node_list[2].push_back(off_node_t(-i,j));
				off_node_list[5].push_back(off_node_t(-i,-j));
				off_node_list[6].push_back(off_node_t(i,-j));
			}
	}
	~MoveAgent()
	{
		for(int i=0; i<MOVEAGENT_ROW; i++)
			for(int j=0; j<MOVEAGENT_COL; j++)
				if(reachable_table[i][j]) delete reachable_table[i][j];
		
		for(int i=0; i<MOVEAGENT_ROW; i++)
			delete[] reachable_table[i];
		delete[] reachable_table;
	}

private:
	bool Locate(const A3DVECTOR & pos, int & row, int & col);

private:
	SpinLock lock;
	A3DVECTOR *** reachable_table;

	struct off_node_t
	{
		int x_off;
		int z_off;
		off_node_t(int x,int z):x_off(x),z_off(z){}
	};
/*
  \  2 | 1  /
	\  |  /
 3    \|/    0
---------------
 4    /|\    7
    /  |  \
  /	 5 |  6 \
*/
	std::vector<off_node_t> off_node_list[8];
};




#endif
